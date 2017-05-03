#include "resource.h"

size_t calculate_memory(resource_config c, size_t available_memory, float panic_factor) {
    size_t default_reserve_memory = std::max<size_t>(1 << 30, 0.05 * available_memory) * panic_factor;
    auto reserve = SHOULDOR(c.reserve_memory, default_reserve_memory);
    size_t min_memory = 200 * (1 << 20);
    if (available_memory >= reserve + min_memory) {
        available_memory -= reserve;
    } else {
        // Allow starting up even in low memory configurations (e.g. 2GB boot2docker VM)
        available_memory = min_memory;
    }
    size_t mem = SHOULDOR(c.total_memory, available_memory);
    if (mem > available_memory) {
        throw std::runtime_error("insufficient physical memory");
    }
    return mem;
}

