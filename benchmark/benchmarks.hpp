#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include <string>
#include <chrono>
#include <vector>
#include <thread>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <atomic>
#include "ring_buffer.hpp"

class RingBufferBenchmark {
private:
    static constexpr size_t TEST_ITERATIONS = 1000000000; // 1B iterations
    
public:
    struct BenchmarkResults {
        double avg_latency_ns;
        double throughput_ops_per_sec;
        size_t total_operations;
        double duration_ms;
    };
    
    // Core benchmark functions
    BenchmarkResults benchmark_single_threaded_latency();
    BenchmarkResults benchmark_producer_consumer_throughput();
    
    // Utility functions
    void print_results(const BenchmarkResults& results, const std::string& test_name);
    void run_all_benchmarks();
    void show_system_infomation();
};

#endif // BENCHMARK_HPP
