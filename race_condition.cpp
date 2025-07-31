// =========================================================
//  * Compile once as‑is to demonstrate data race
//  * Re‑compile with -DUSE_ATOMIC or -DUSE_MUTEX for fixes
// =========================================================
#include <bits/stdc++.h>
#include <thread>
#ifdef USE_ATOMIC
std::atomic<int> counter{0};
#elif defined(USE_MUTEX)
int counter=0; std::mutex m;
#else
int counter=0; // UNSAFE
#endif

void inc(){
    for(int i=0;i<1'000'000;++i){
#ifdef USE_ATOMIC
        counter.fetch_add(1,std::memory_order_relaxed);
#elif defined(USE_MUTEX)
        std::lock_guard<std::mutex> lg(m);
        ++counter;
#else
        ++counter; // data race
#endif
    }
}
int main(){
    auto t0=clk::now();
    std::thread t1(inc),t2(inc),t3(inc),t4(inc);
    t1.join();t2.join();t3.join();t4.join();
    auto dt=dur(clk::now()-t0).count();
    std::cout<<"counter="<<counter<<" (expected 4M) in "<<dt<<" ms\n";
}
