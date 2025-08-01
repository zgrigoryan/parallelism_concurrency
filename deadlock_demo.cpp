#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <iomanip>

using clk = std::chrono::steady_clock;
using dur = std::chrono::duration<double, std::milli>;

std::mutex m1, m2;

void a() {
    std::lock_guard<std::mutex> lg1(m1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
#ifndef FIX
    std::lock_guard<std::mutex> lg2(m2);
#else
    std::scoped_lock sl(m1, m2);
#endif
}

void b() {
#ifndef FIX
    std::lock_guard<std::mutex> lg2(m2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lg1(m1);
#else
    std::scoped_lock sl(m1, m2);
#endif
}

int main() {
    std::cout << "Starting… (Ctrl+C to break potential deadlock)\n";
    std::thread t1(a), t2(b);
    t1.join();
    t2.join();
    std::cout << "Completed without deadlock.\n";
}
