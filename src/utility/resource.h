#ifndef XHBLIB_UTILITY_CPUID_H_
#define XHBLIB_UTILITY_CPUID_H_

#if !(defined(__linux__) ||  defined(__APPLE__) || defined(_WIN32))
#error "os not support"
#endif

#include <vector>
#include <set>

struct resource_config {
    size_t total_memory;
    size_t reserve_memory;
    unsigned int cpus;
    std::set<unsigned int> cpu_set;
    size_t max_io_request;
    unsigned int io_queues;
};

struct memory_t {
    unsigned int node_id;
    size_t bytes;
};

struct io_queue_t {
    unsigned int id;
    size_t capacity;
};

struct io_queue_topology_t {
    std::vector<unsigned int> shard_to_coordinator;
    std::vector<io_queue_t> coordinators;
};

struct cpu_t {
    unsigned int cpu_id;
    std::vector<memory> mems;
};

struct resource_t {
    std::vector<cpu_t> cpus;
    io_queue_topology_t io_queues;
};

resource_t allocate_resource(const resource_config& config);

unsigned int get_pu_count();

int get_current_cpuid();

cup_set_t get_cpuset(unsigned int cpuid);

#endif // XHBLIB_UTILITY_CPUID_H_
