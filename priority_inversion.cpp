#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#ifdef FAST_CI
constexpr int kHoldMs = 20;
#else
constexpr int kHoldMs = 200;
#endif

#ifdef _WIN32   // ───────────────────────────────────────── Windows stub
int main() {
    std::cout << "priority_inversion demo is POSIX-only – skipped on Windows.\n";
    return 0;
}
#else           // ───────────────────────────────────────── POSIX version

#include <unistd.h>
#ifdef __linux__
#include <pthread.h>
#endif

using clk = std::chrono::steady_clock;
std::mutex shared_mtx;
std::atomic<bool> done{false};

auto now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        clk::now().time_since_epoch()).count();
}

void set_prio(int prio) {
#ifdef __linux__
    sched_param sp{ .sched_priority = prio };
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp);
#else
    (void)prio;
#endif
}

void low_thread() {
    set_prio(10);
    std::scoped_lock lg(shared_mtx);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // hold resource
}

void high_thread() {
    set_prio(80);
    auto t0 = now_ms();
    std::scoped_lock lg(shared_mtx);
    std::cout << "High acquired after " << now_ms() - t0 << " ms\n";
    done = true;
}

void medium_thread() {
    set_prio(50);
    while (!done) std::this_thread::yield();
}

int main() {
    std::cout << "Simulating priority inversion…\n";
    std::thread L(low_thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::thread H(high_thread);
    std::thread M(medium_thread);
    L.join(); H.join(); M.join();
    std::cout << "Complete.  Re-build with -DPRIO_INHERIT and a PI mutex for the real fix.\n";
}

#endif  // _WIN32
