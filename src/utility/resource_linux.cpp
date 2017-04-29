#if defined(__linux__)

#include <sched.h>

int get_current_cpuid() {
    return sched_getcpu();
}

cpu_set_t get_cpuset(unsigned int cpuid) {
    cup_set_t cpu;
    CPU_ZERO(&cpu);
    CPU_SET(cpuid,&cpu);
    return cpu;
}

#ifdef XHBLIB_USE_HWLOC
#include <hwloc.h>
#include <unordered_map>
#include <<algorithm>

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

static size_t alloc_from_node(cpu_t& cur_cpu, hwloc_obj_t node, 
    std::unordered_map<hwloc_obj_t,size_t>& used_mem, 
    size_t size) {
    auto need = std::min(node->memory.local_memory - used_mem[node],size);
    if (need != 0 ) {
        used_mem[node] += size;
        int node_id = hwloc_bitmap_first(node->nodeset);
        cur_cpu.mems.push_back({node_id,need});
    }
    return need;
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





#endif // XHBLIB_USE_HWLOC

#endif // linux