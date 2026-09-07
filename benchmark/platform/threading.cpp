#include "threading.hpp"
#include <iostream>
#include <pthread.h>
#include <sched.h>

static std::string get_policy_name(int policy)
{
    std::string policyName;
    switch (policy)
    {
    case SCHED_OTHER:
        policyName = "SCHED_OTHER";
        break;
    case SCHED_FIFO:
        policyName = "SCHED_FIFO";
        break;
    case SCHED_RR:
        policyName = "SCHED_RR";
        break;
    default:
        policyName = std::to_string(policy);
        break;
    }
    return policyName;
}

namespace platform
{
    bool pin_thread_to_core(int core_id)
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core_id, &cpuset);

        int result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
        if (result != 0)
        {
            printf("pthread_setaffinity_np failed, result=%d, core=%d\n", result, core_id);
            return false;
        }
        return true;
    }

    bool set_high_thread_priority()
    {
        int policy = SCHED_FIFO;
        sched_param sp;
        sp.sched_priority = sched_get_priority_max(policy);
        const int ret = pthread_setschedparam(pthread_self(), policy, &sp);
        if (ret == 0)
        {
            printf("pthread_setschedparam success, policy=%s, priority:%d\n", get_policy_name(policy).c_str(), sp.sched_priority);
            return true;
        }
        else
        {
            printf("pthread_setschedparam failed, ret=%d, policy=%s, priority:%d\n", ret, get_policy_name(policy).c_str(), sp.sched_priority);
            return false;
        }
    }

} // namespace platform
