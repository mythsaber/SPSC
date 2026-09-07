#ifndef PLATFORM_THREADING_HPP
#define PLATFORM_THREADING_HPP

namespace platform {
    bool pin_thread_to_core(int core_id);
    bool set_high_thread_priority();
}

#endif // PLATFORM_THREADING_HPP
