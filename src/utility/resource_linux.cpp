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

#ifndef XHBLIB_USE_HWLOC

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