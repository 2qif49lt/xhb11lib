#include "resource.h"

#ifdef __APPLE__ 

#include <cpuid.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#define _xhb_CPUID(INFO, LEAF, SUBLEAF) __cpuid_count(LEAF, SUBLEAF, INFO[0], INFO[1], INFO[2], INFO[3])

int get_current_cpuid() {
    int id = 0;

    unsigned int CPUInfo[4];
    _xhb_CPUID(CPUInfo, 1, 0);
    /* CPUInfo[1] is EBX, bits 24-31 are APIC ID */ 
    if ( (CPUInfo[3] & (1 << 9)) == 0) { 
        id =  -1;  /* no APIC on chip */ 
    } 
    else { 
        id =  (unsigned)CPUInfo[1] >> 24; 
    } 
    if (id < 0) id = 0;
    return id;
}

static inline void CPU_ZERO(cpu_set_t *cs) { cs->count = 0; }

static inline void CPU_SET(int num, cpu_set_t *cs) { cs->count |= (1 << num); }

static inline int CPU_ISSET(int num, cpu_set_t *cs) { return (cs->count & (1 << num)); }

cpu_set_t get_cpuset(unsigned int cpuid) {
    cpu_set_t cpu;
    CPU_ZERO(&cpu);
    CPU_SET(cpuid,&cpu);
    return cpu;
}

#ifndef XHBLIB_USE_HWLOC

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
    auto cpuset_procs = SHOULDOR(c.cpu_set.size(), get_pu_count());
    auto procs = SHOULDOR(c.cpus, cpuset_procs);
    ret.cpus.reserve(procs);
    for (unsigned i = 0; i < procs; ++i) {
        ret.cpus.push_back({i, {{0, mem / procs}}});
    }

    ret.io_queues = allocate_io_queues(c, ret.cpus);
    return ret;
}

unsigned int get_pu_count() {
    return ::sysconf(_SC_NPROCESSORS_ONLN);
}


#endif // XHBLIB_USE_HWLOC
#endif // apple