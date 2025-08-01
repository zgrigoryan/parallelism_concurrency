#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <iomanip>

using clk = std::chrono::steady_clock;
using dur = std::chrono::duration<double, std::milli>;

void test_atomic(unsigned threads, unsigned iters) {
    std::atomic<int> c{0};
    auto t0 = clk::now();
    std::vector<std::thread> w;
    for (unsigned i = 0; i < threads; ++i)
        w.emplace_back([&] {
            for (unsigned j = 0; j < iters; ++j)
                c.fetch_add(1, std::memory_order_relaxed);
        });
    for (auto &t : w) t.join();
    auto dt = dur(clk::now() - t0).count();
    std::cout << "atomic  " << std::setw(2) << threads
              << "T -> " << std::fixed << std::setprecision(2)
              << dt << " ms\n";
}

void test_mutex(unsigned threads, unsigned iters) {
    int c = 0;
    std::mutex m;
    auto t0 = clk::now();
    std::vector<std::thread> w;
    for (unsigned i = 0; i < threads; ++i)
        w.emplace_back([&] {
            for (unsigned j = 0; j < iters; ++j) {
                std::lock_guard<std::mutex> lg(m);
                ++c;
            }
        });
    for (auto &t : w) t.join();
    auto dt = dur(clk::now() - t0).count();
    std::cout << "mutex   " << std::setw(2) << threads
              << "T -> " << std::fixed << std::setprecision(2)
              << dt << " ms\n";
}

int main() {
    const unsigned N = 1'000'000;
    for (unsigned th : {1, 2, 4, 8, 16}) {
        test_atomic(th, N);
        test_mutex(th, N);
        std::cout << "--\n";
    }
}
