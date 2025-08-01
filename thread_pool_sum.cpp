#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <numeric>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iomanip>

using clk = std::chrono::steady_clock;
using dur = std::chrono::duration<double, std::milli>;
class barrier {
    std::mutex m;
    std::condition_variable cv;
    int count = 0;
    const int total;
public:
    explicit barrier(int n) : total(n) {}
    void arrive_and_wait() {
        std::unique_lock lock(m);
        ++count;
        if (count < total) {
            cv.wait(lock, [&] { return count >= total; });
        } else {
            cv.notify_all();
        }
    }
};


class ThreadPool {
    struct task_wrapper {
        std::function<void()> fn;
    };
    std::vector<std::thread> workers;
    std::queue<task_wrapper> q;
    std::mutex m;
    std::condition_variable cv;
    std::atomic<bool> stop = false;
public:
    explicit ThreadPool(unsigned n = std::thread::hardware_concurrency()) {
        for (unsigned i = 0; i < n; ++i) {
            workers.emplace_back([this] { this->worker(); });
        }
    }
    ~ThreadPool() {
        stop = true; cv.notify_all();
        for (auto &w : workers) w.join();
    }
    template<typename F>
    void enqueue(F&& f) {
        {
            std::lock_guard<std::mutex> lg(m);
            q.push({std::forward<F>(f)});
        }
        cv.notify_one();
    }
private:
    void worker() {
        while (true) {
            task_wrapper task;
            {
                std::unique_lock<std::mutex> ul(m);
                cv.wait(ul, [this] { return stop || !q.empty(); });
                if (stop && q.empty()) return;
                task = std::move(q.front()); q.pop();
            }
            task.fn();
        }
    }
};

std::uint64_t pool_sum(const std::vector<int>& v, ThreadPool &pool, unsigned n_tasks) {
    const std::size_t n = v.size();
    std::vector<std::uint64_t> partial(n_tasks);
    barrier sync(n_tasks + 1);
    const std::size_t chunk = (n + n_tasks - 1) / n_tasks;
    for (unsigned t = 0; t < n_tasks; ++t) {
        const std::size_t start = t * chunk;
        const std::size_t stop = std::min(start + chunk, n);
        pool.enqueue([&, t, start, stop] {
            partial[t] = std::accumulate(v.begin() + start, v.begin() + stop, 0ull);
            sync.arrive_and_wait();
        });
    }
    sync.arrive_and_wait(); // main thread waits
    return std::accumulate(partial.begin(), partial.end(), 0ull);
}

int main() {
    const std::size_t N = 50'000'000;
    auto data = std::vector<int>(N, 1);
    ThreadPool pool;
    for (unsigned tasks : {1, 2, 4, 8, 16, 32}) {
        auto t0 = clk::now();
        auto s = pool_sum(data, pool, tasks);
        auto dt = dur(clk::now() - t0).count();
        std::cout << "tasks " << std::setw(2) << tasks
                  << ": sum " << s
                  << " -> " << std::fixed << std::setprecision(2) << dt << " ms\n";
    }
}
