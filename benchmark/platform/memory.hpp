#ifndef PLATFORM_MEMORY_HPP
#define PLATFORM_MEMORY_HPP

#include <cstddef>

namespace platform {    
    int get_cpu_count();
    size_t get_available_memory();
    size_t get_cache_line_size();

    void prefault_memory(void* ptr, size_t size);
    bool lock_memory(void* ptr, size_t size);
    bool unlock_memory(void* ptr, size_t size);
}

#endif // PLATFORM_MEMORY_HPP
