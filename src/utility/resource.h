#ifndef XHBLIB_UTILITY_CPUID_H_
#define XHBLIB_UTILITY_CPUID_H_

#if !(defined(__linux__) ||  defined(__APPLE__) || defined(_WIN32))
#error "os not support"
#endif

#include <vector>
#include <set>

struct resource_config {
    size_t total_memory; // 用户设置的最大可用内存,如果为0,则为系统最大内存
    size_t reserve_memory; // 用户设置的保留的内存,如果为0，则内部计算出合适的保留内存
    unsigned int cpus; // 用户设置的占用几个cpu，如果为0，则依赖cpu_set.
    std::set<unsigned int> cpu_set; // 用户设置的希望绑定的cpu.如果为空，则为系统当前所有的cpu
    unsigned int max_io_request; // 单个队列最大请求数，可以为0默认128*队列数
    unsigned int io_queues; // 队列数，为0则为占用的cpu数
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

size_t calculate_memory(resource_config c, size_t available_memory, float panic_factor = 1);

resource_t allocate_resource(const resource_config& c);

unsigned int get_pu_count();

int get_current_cpuid();

#define SHOULDOR(set_val, def_val) ((set_val != 0) ? (set_val) : (def_val))

#if defined(__APPLE__)

struct cpu_set_t {
  unsigned int count;
};

#endif

cpu_set_t get_cpuset(unsigned int cpuid);

#endif // XHBLIB_UTILITY_CPUID_H_
