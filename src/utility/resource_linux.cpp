#if defined(__linux__)
#include "resource.h"
#include <sched.h>

int get_current_cpuid() {
    return sched_getcpu();
}

cpu_set_t get_cpuset(unsigned int cpuid) {
    cpu_set_t cpu;
    CPU_ZERO(&cpu);
    CPU_SET(cpuid,&cpu);
    return cpu;
}

#ifdef XHBLIB_USE_HWLOC
#include <hwloc.h>
#include <unordered_map>
#include <<algorithm>
#include <tuple>
#include "defer.h"
#include "align.h"
// 获得node的深度，node包含local 内存
// 计算单元的祖辈节点都包含内存 pu < core == L1 == L2 == L3(package内core共享) < Package == NUMA node < machine
// == 表示兄弟节点，< 表示属于子节点关系
static int get_memory_depth(hwloc_topology_t topo) {
    int depth =  hwloc_get_type_depth(topo, HWLOC_OBJ_PU);
    if (depth != HWLOC_TYPE_DEPTH_UNKNOWN) {
        hwloc_obj_t obj = hwloc_get_obj_by_depth(topo, depth, 0);

        while (obj && obj->memory.local_memory) {
            obj = hwloc_get_ancestor_obj_by_depth(topo, --depth, obj);
        }
    }
    return depth;
}

// 在cpu对应的node预分配,size为启动时可用总内存/pu个数
static size_t alloc_from_node(cpu_t& cur_cpu, hwloc_obj_t node, 
    std::unordered_map<hwloc_obj_t,size_t>& used_mem, 
    size_t size) {
    auto need = std::min(node->memory.local_memory - used_mem[node], size);
    if (need != 0 ) {
        used_mem[node] += need;
        int node_id = hwloc_bitmap_first(node->nodeset);
        cur_cpu.mems.push_back({node_id,need});
    }
    return need;
}

// 
struct distribute_objects {
    std::vector<hwloc_cpuset_t> cpu_sets;
    hwloc_obj_t root;

    distribute_objects(hwloc_topology_t& topo, size_t nobjs) : cpu_sets(nobjs), 
        //hwloc_get_root_obj 一般情况总是 HWLOC_OBJ_MACHINE 
        root(hwloc_get_root_obj(topo)) {
        hwloc_distrib(topo, &root, 1, cpu_sets.data(), cpu_sets.size(), INT_MAX, 0);
    }

    ~distribute_objects() {
        for (auto&& cs : cpu_sets) {
            hwloc_bitmap_free(cs);
        }
    }
    std::vector<hwloc_cpuset_t>& operator()() {
        return cpu_sets;
    }
};


static io_queue_topology_t
allocate_io_queues(hwloc_topology_t topo, resource_config c, vector<cpu_t> cpus) {
    unsigned int num_io_queues = SHOULDOR(c.io_queues, cpus.size());
    if (num_io_queues > cpus.size()) num_io_queues = cpus.size();

    unsigned int num_max_io_request = SHOULDOR(c.max_io_request, 128 * num_io_queues);

    unsigned int depth = get_memory_depth(topo);

    // 由index cpu得到所归属的numa node id
    auto nodeid_of_shard = [&topo, &cpus, &depth] (unsigned int shard) {
        auto pu = hwloc_get_pu_obj_by_os_index(topo, cpus[shard].cpu_id);
        auto node = hwloc_get_ancestor_obj_by_depth(topo, depth, pu);
        return hwloc_bitmap_first(node->nodeset);
    };

    // 将cpu index 按node归属关系分类
    std::unordered_map<unsigned int /*node id*/, std::set<unsigned int> /*cpu index*/> numa_nodes;
    for (unsigned int shard = 0; shard != cpu.size(); ++shard) {
        auto node_id = nodeid_of_shard(shard);

        if (numa_nodes.count(node_id) == 0) {
            numa_nodes.emplace(node_id, std::set<unsigned int>());
        }
        numa_nodes.at(node_id).insert(shard);
    }

    io_queue_topology_t ret;
    ret.shard_to_coordinator.resize(cpus.size());

    // 根据cpu id 获取到 cpu index
    auto shardid_of_cpuid = [&cpus] (unsigned int cpu_id) {
        auto idx = 0u;
        for (auto& c: cpus) {
            if (c.cpu_id == cpu_id) {
                return idx;
            }
            idx++;
        }
        assert(0);
    };

    auto cpu_sets = distribute_objects(topo, num_io_queues);
    // First step: distribute the IO queues given the information returned in cpu_sets.
    // If there is one IO queue per processor, only this loop will be executed.
    // 分配到的node id 与 cpu index的对应集合
    std::unordered_map<unsigned int /*node id*/, std::vector<unsigned> /*cpu index*/> node_coordinators;
    for (auto&& cs : cpu_sets()) {
        auto io_coordinator = shardid_of_cpuid(hwloc_bitmap_first(cs));

        ret.coordinators.emplace_back(io_queue_t{io_coordinator, std::max(num_max_io_request / num_io_queues , 1u)});
        // If a processor is a coordinator, it is also obviously a coordinator of itself
        ret.shard_to_coordinator[io_coordinator] = io_coordinator;

        auto node_id = nodeid_of_shard(io_coordinator);
        if (node_coordinators.count(node_id) == 0) {
            node_coordinators.emplace(node_id, std::vector<unsigned int>());
        }
        node_coordinators.at(node_id).push_back(io_coordinator);
        numa_nodes[node_id].erase(io_coordinator);
    }

    // If there are more processors than coordinators, we will have to assign them to existing
    // coordinators. We always do that within the same NUMA node.
    for (auto& node: numa_nodes) {
        auto cid_idx = 0;
        for (auto& remaining_shard: node.second) {
            auto idx = cid_idx++ % node_coordinators.at(node.first).size();
            auto io_coordinator = node_coordinators.at(node.first)[idx];
            ret.shard_to_coordinator[remaining_shard] = io_coordinator;
        }
    }

    return ret;

}



resource_t allocate_resource(const resource_config& c) {
    hwloc_topology_t topo;
    hwloc_topology_init(&topo);
    defer([&topo] { hwloc_topology_destroy(topo); }); 

    hwloc_topology_load(topo);
    if (c.cpu_set.size()) {
        auto bm = hwloc_bitmap_alloc();
        defer([&bm] { hwloc_bitmap_free(bm); });
        
        for (auto idx : c.cpu_set) {
            hwloc_bitmap_set(bm, idx);
        }
        
        // 修改拓扑移除不在bm中的cpus.
        auto r = hwloc_topology_restrict(topo, bm,
                HWLOC_RESTRICT_FLAG_ADAPT_DISTANCES
                | HWLOC_RESTRICT_FLAG_ADAPT_MISC
                | HWLOC_RESTRICT_FLAG_ADAPT_IO);
        if (r == -1) {
            if (errno == ENOMEM) {
                throw std::bad_alloc();
            }
            if (errno == EINVAL) {
                throw std::runtime_error("bad cpuset");
            }
            abort();
        }
    }
    auto machine_depth = hwloc_get_type_depth(topo, HWLOC_OBJ_MACHINE);
    //   assert(hwloc_get_type_depth(topology, HWLOC_OBJ_MACHINE) == 1);
    //   assert(hwloc_get_nbobjs_by_depth(topo, machine_depth) == 1);

    // 计算可使用内存大小
    auto machine = hwloc_get_obj_by_depth(topo, machine_depth, 0);
    auto available_memory = machine->memory.total_memory;
    // hwloc doesn't account for kernel reserved memory, so set panic_factor = 2
    size_t mem = calculate_memory(c, available_memory, 2);

    // 获取cpu数
    unsigned int available_procs = (unsigned int)hwloc_get_nbobjs_by_type(topo, HWLOC_OBJ_PU);
    unsigned int procs = SHOULDOR(c.cpus, available_procs);
    if (procs > available_procs) {
        throw std::runtime_error("insufficient processing units");
    }
    auto mem_per_proc = align_down<2 << 20>(mem / procs);

    resource_t ret;
    std::unordered_map<hwloc_obj_t, size_t> topo_used_mem;
    std::vector<std::pair<cpu_t, size_t>> remains;
    size_t remain;
    unsigned depth = get_memory_depth(topo);

    auto cpu_sets = distribute_objects(topo, procs);

    // Divide local memory to cpus
    for (auto&& cs : cpu_sets()) {
        auto cpu_id = hwloc_bitmap_first(cs);
        assert(cpu_id != -1);
        auto pu = hwloc_get_pu_obj_by_os_index(topo, cpu_id);
        auto node = hwloc_get_ancestor_obj_by_depth(topo, depth, pu);
        cpu_t this_cpu;
        this_cpu.cpu_id = cpu_id;
        remain = mem_per_proc - alloc_from_node(this_cpu, node, topo_used_mem, mem_per_proc);

        remains.emplace_back(std::move(this_cpu), remain);
    }

    // Divide the rest of the memory
    // 如果在当前node不够内存，则在兄弟node进行分配
    for (auto&& r : remains) {
        cpu_t this_cpu;
        size_t remain;
        std::tie(this_cpu, remain) = r;
        auto pu = hwloc_get_pu_obj_by_os_index(topo, this_cpu.cpu_id);
        auto node = hwloc_get_ancestor_obj_by_depth(topo, depth, pu);
        auto obj = node;

        while (remain) {
            remain -= alloc_from_node(this_cpu, obj, topo_used_mem, remain);
            do {
                obj = hwloc_get_next_obj_by_depth(topo, depth, obj);
            } while (!obj);
            if (obj == node)
                break;
        }
        assert(!remain);
        ret.cpus.push_back(std::move(this_cpu));
    }

    ret.io_queues = allocate_io_queues(topo, c, ret.cpus);
    return ret;
}

unsigned int get_pu_count() {
    hwloc_topology_t topo;
    hwloc_topology_init(&topo);
    hwloc_topology_load(topo);
    int width =  hwloc_get_nbobjs_by_type(topo, HWLOC_OBJ_PU);
    hwloc_topology_destroy(topo); 
    return width;
}



#else

#include <unistd.h>

static io_queue_topology_t
allocate_io_queues(resource_config c, std::vector<cpu_t> cpus) {
    io_queue_topology_t ret;

    unsigned int nr_cpus = (unsigned int)cpus.size();
    unsigned int max_io_requests = SHOULDOR(c.max_io_request, 128 * nr_cpus);

    ret.shard_to_coordinator.resize(nr_cpus);
    ret.coordinators.resize(nr_cpus);

    for (unsigned int shard = 0; shard < nr_cpus; ++shard) {
        ret.shard_to_coordinator[shard] = shard;
        ret.coordinators[shard].capacity =  std::max(max_io_requests / nr_cpus, 1u);
        ret.coordinators[shard].id = shard;
    }
    return ret;
}


resource_t allocate_resource(const resource_config& c) {
    resource_t ret;

    auto available_memory = ::sysconf(_SC_PAGESIZE) * size_t(::sysconf(_SC_PHYS_PAGES));
    auto mem = calculate_memory(c, available_memory);
    auto cpuset_procs = SHOULDOR(c.cpu_set.size(), get_pu_count();
    auto procs = SHOULDOR(c.cpus, cpuset_procs);
    ret.cpus.reserve(procs);
    for (unsigned i = 0; i < procs; ++i) {
        ret.cpus.push_back({i, {0, mem / procs}});
    }

    ret.io_queues = allocate_io_queues(c, ret.cpus);
    return ret;
}

unsigned int get_pu_count() {
    return ::sysconf(_SC_NPROCESSORS_ONLN);
}


#endif // XHBLIB_USE_HWLOC

#endif // linux