#include "benchmarks.hpp"
#include "platform/threading.hpp"
#include "platform/memory.hpp"
#include <iostream>
#include <exception>

int main() {
    try {
        std::cout << "Starting SPSC Ring Buffer Benchmark Suite..." << std::endl;
        std::cout << "=============================================" << std::endl;
        
        // Create benchmark instance and run
        RingBufferBenchmark benchmark;
        benchmark.run_all_benchmarks();
        
        std::cout << "\n=============================================" << std::endl;
        std::cout << "Benchmark suite completed successfully!" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error running benchmarks: " << e.what() << std::endl;
        return 1;
        
    } catch (...) {
        std::cerr << "Unknown error occurred during benchmarking!" << std::endl;
        return 2;
    }
}
