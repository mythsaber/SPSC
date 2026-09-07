#include "benchmarks.hpp"
#include "platform/threading.hpp"
#include "platform/memory.hpp"
#include <mutex>
#include <immintrin.h>

struct GoodsMessage
{
    uint64_t timestamp;
    uint32_t symbol_id;
    uint32_t price;
    uint32_t quantity;
    char side;
};

constexpr size_t QUEUE_SIZE = 65536;

void RingBufferBenchmark::run_all_benchmarks()
{
    show_system_infomation();

    std::cout << std::endl;
    std::cout << "running single-threaded latency benchmark. . ." << std::endl;
    auto latency_results = benchmark_single_threaded_latency();
    print_results(latency_results, "Single-Threaded Latency");

    std::cout << std::endl;
    std::cout << "running producer-consumer throughput benchmark. . ." << std::endl;
    auto throughput_results = benchmark_producer_consumer_throughput();
    print_results(throughput_results, "Producer-Consumer Throughput");
}

void RingBufferBenchmark::show_system_infomation()
{
    std::cout << "\n"
              << "=== System Infomation ===" << std::endl;

    std::cout << "CPU cores: " << platform::get_cpu_count() << std::endl;
    std::cout << "cache line: " << platform::get_cache_line_size() << " Byte" << std::endl;
    std::cout << "available memory: " << (platform::get_available_memory() / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "message size: " << sizeof(GoodsMessage) << " bytes" << std::endl;
    std::cout << "messages per cache line: " << platform::get_cache_line_size() / sizeof(GoodsMessage) << std::endl;
    std::cout << "ring buffer size: " << sizeof(GoodsMessage) * QUEUE_SIZE / 1024 << " kByte" << std::endl;

    std::cout << std::endl;
}

RingBufferBenchmark::BenchmarkResults RingBufferBenchmark::benchmark_single_threaded_latency()
{
    // Warmup and measure operations...
    std::vector<double> latencies;
    latencies.reserve(TEST_ITERATIONS);

    SPSCQueue<GoodsMessage, QUEUE_SIZE> queue;
    platform::prefault_memory(queue.internal_buffer(), queue.internal_buffer_size());
    GoodsMessage msg, popped;

    // Measure latencies
    auto start_time = std::chrono::steady_clock::now();
    for (size_t i = 0; i < TEST_ITERATIONS; ++i)
    {
        queue.push(msg);
        queue.pop(popped);
    }
    asm volatile("" : : "g"(popped) : "memory");
    auto end_time = std::chrono::steady_clock::now();
    double duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                             end_time - start_time)
                             .count();

    BenchmarkResults results;
    results.total_operations = TEST_ITERATIONS;
    results.duration_ms = duration_ns / 1000000;
    results.throughput_ops_per_sec = (results.total_operations / duration_ns) * 1000000000.0;
    results.avg_latency_ns = duration_ns / results.total_operations;

    return results;
}

RingBufferBenchmark::BenchmarkResults RingBufferBenchmark::benchmark_producer_consumer_throughput()
{
    SPSCQueue<GoodsMessage, QUEUE_SIZE> queue;
    platform::prefault_memory(queue.internal_buffer(), queue.internal_buffer_size());
    std::atomic<bool> start_flag{false};
    std::chrono::steady_clock::time_point end_time;

    GoodsMessage msg;
    msg.timestamp = 123456789;
    msg.symbol_id = 12345;
    msg.price = 15050;
    msg.quantity = 100;
    msg.side = 'B';

    std::thread producer([&queue, &msg, &start_flag]()
                         {
        const int core = 10;
        printf("created producer thread (pinned to core %d)...\n", core);
        platform::pin_thread_to_core(core);  // avoid core 0 (OS heavy)
        platform::set_high_thread_priority();
        
        while (!start_flag.load(std::memory_order_acquire)) { _mm_pause(); } // 等待统一开始

        for (size_t i = 0; i < TEST_ITERATIONS; ++i) {
            while (!queue.push(msg)) {
                _mm_pause();
            }
        } });

    std::thread consumer([&queue, &start_flag, &end_time]()
                         {
        const int core = 11;
        printf("created consumer thread (pinned to core %d)...\n", core);
        platform::pin_thread_to_core(core);  // separate core from producer
        platform::set_high_thread_priority();
        
        while (!start_flag.load(std::memory_order_acquire)) { _mm_pause(); } // 等待统一开始

        GoodsMessage popped;
        for (size_t i = 0; i < TEST_ITERATIONS; ++i) {
            while (!queue.pop(popped)) {
                _mm_pause();
            }
        }

        end_time = std::chrono::steady_clock::now();
        asm volatile("" : : "g"(popped) : "memory"); });

    std::cout << "Waiting for threads to complete..." << std::endl;
    auto start_time = std::chrono::steady_clock::now();
    start_flag.store(true, std::memory_order_release);
    producer.join();
    consumer.join();

    double duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();

    BenchmarkResults results;
    results.total_operations = TEST_ITERATIONS;
    results.duration_ms = duration_ns / 1000000.0;
    results.throughput_ops_per_sec = (results.total_operations / duration_ns) * 1000000000.0;
    results.avg_latency_ns = duration_ns / results.total_operations;

    return results;
}

void RingBufferBenchmark::print_results(const BenchmarkResults &results, const std::string &test_name)
{
    std::cout << test_name << " Results:" << std::endl;
    std::cout << "+---------------------------------------+" << std::endl;
    std::cout << "| Operations: " << std::setw(10) << results.total_operations << "           |" << std::endl;
    std::cout << "| Duration: " << std::setw(12) << std::fixed << std::setprecision(2) << results.duration_ms << " ms      |" << std::endl;
    std::cout << "| Throughput: " << std::setw(10) << std::fixed << std::setprecision(0) << results.throughput_ops_per_sec << " ops/sec  |" << std::endl;
    std::cout << "| Avg Latency: " << std::setw(8) << std::fixed << std::setprecision(1) << results.avg_latency_ns << " ns        |" << std::endl;

    std::cout << "+---------------------------------------+" << std::endl;
}
