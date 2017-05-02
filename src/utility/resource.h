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
    unsigned int max_io_request; // 单个队列最大请求数，可以为0默认128*队列数
    unsigned int io_queues; // 队列数，为人则为cpu数
};

struct memory_t {
    unsigned int node_id;
    size_t bytes;
};

struct io_queue_t {
    unsigned int id; // cpu index id
    size_t capacity;
};

struct io_queue_topology_t {
    std::vector<unsigned int> shard_to_coordinator;
    std::vector<io_queue_t> coordinators;
};

struct cpu_t {
    unsigned int cpu_id; //物理id
    std::vector<memory_t> mems;
};

struct resource_t {
    std::vector<cpu_t> cpus;
    io_queue_topology_t io_queues;
};

resource_t allocate_resource(const resource_config& c);

unsigned int get_pu_count();

int get_current_cpuid();

cup_set_t get_cpuset(unsigned int cpuid);

#endif // XHBLIB_UTILITY_CPUID_H_
