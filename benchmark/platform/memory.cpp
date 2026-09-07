#include "memory.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>

namespace platform
{
    int get_cpu_count()
    {
        return static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
    }

    size_t get_cache_line_size()
    {
        auto size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        return static_cast<std::size_t>(size);
    }

    size_t get_available_memory()
    {
        struct sysinfo info;
        if (sysinfo(&info) == 0)
        {
            return static_cast<size_t>(info.freeram * info.mem_unit);
        }
        return 0;
    }

    void prefault_memory(void* ptr, size_t size)
    {
        // 调用 madvise 告知内核此内存区域即将被访问，建议预先载入物理页
        int result = madvise(ptr, size, MADV_WILLNEED);
        if (result == 0)
        {
            std::cout << "madvise(MADV_WILLNEED) 执行成功，已向内核发起内存预加载建议。" << std::endl;
        }
        else
        {
            // 失败时记录 errno 并输出具体的错误原因
            int err = errno;
            std::cerr << "madvise 失败! 错误码: " << err << " (" << std::strerror(err) << ")" << std::endl;
            return;
        }
    }

    bool lock_memory(void *ptr, size_t size)
    {
        (void)ptr;
        (void)size;
        std::cerr << "Memory locking not implemented" << std::endl;
        return false;
    }

    bool unlock_memory(void *ptr, size_t size)
    {
        (void)ptr;
        (void)size;
        return false;
    }

} // namespace platform
