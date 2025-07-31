
// =========================================================
//  * Parallel reduction with std::thread
//  * Measures speed-up with growing thread count
//  * Measures pure create/join overhead
// =========================================================
#include <iostream>
#include <vector>
#include <numeric>
#include <thread>
#include <chrono>
#include <format>      // C++20
#include <algorithm>
#include <functional>


using clk = std::chrono::steady_clock;
using dur = std::chrono::duration<double, std::milli>;

// --- helpers --------------------------------------------------------------
std::vector<int> make_data(std::size_t n)
{
    std::vector<int> v(n);
    std::iota(v.begin(), v.end(), 1); // 1,2,3,…
    return v;
}

std::uint64_t parallel_sum(const std::vector<int>& v, unsigned n_threads)
{
    const std::size_t n = v.size();
    std::vector<std::thread> workers;
    std::vector<std::uint64_t> partial(n_threads);
    const std::size_t chunk = (n + n_threads - 1) / n_threads;
    for (unsigned t = 0; t < n_threads; ++t) {
        const std::size_t start = t * chunk;
        const std::size_t stop  = std::min(start + chunk, n);
        workers.emplace_back([&, start, stop, t] {
            partial[t] = std::accumulate(v.begin()+start, v.begin()+stop, 0ull);
        });
    }
    for (auto &w : workers) w.join();
    return std::accumulate(partial.begin(), partial.end(), 0ull);
}

void benchmark_sum(std::size_t n)
{
    auto data = make_data(n);
    const unsigned hw = std::thread::hardware_concurrency();
    std::cout << "Elements: " << n << " | cores: " << hw << "\n";

    for (unsigned th = 1; th <= hw*2; th*=2) {
        const auto t0 = clk::now();
        auto s = parallel_sum(data, th);
        const auto dt = dur(clk::now()-t0).count();
        std::cout << std::format("threads {:>2}: sum {:>12} -> {:>8.2f} ms\n", th, s, dt);
    }
}

void measure_thread_overhead(unsigned loops)
{
    const auto t0 = clk::now();
    for (unsigned i=0;i<loops;++i) {
        std::thread([]{}).join();
    }
    std::cout << "Created+joined " << loops << " empty threads in "
              << dur(clk::now()-t0).count() << " ms (" << loops*1000.0/dur(clk::now()-t0).count() << " /s)\n";
}

int main(int argc,char*argv[])
{
    const std::size_t N = (argc>1? std::stoull(argv[1]) : 50'000'000ull);
    benchmark_sum(N);
    measure_thread_overhead(1'000'000);
}