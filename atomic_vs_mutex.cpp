// =========================================================
//  * Micro‑benchmark comparing ++counter guarded by mutex vs atomic.
// =========================================================
#include <bits/stdc++.h>
#include <thread>
#include <atomic>
#include <mutex>

void test_atomic(unsigned threads,unsigned iters){
    std::atomic<int> c{0};
    auto t0=clk::now();
    std::vector<std::thread> w;
    for(unsigned i=0;i<threads;++i) w.emplace_back([&]{for(unsigned j=0;j<iters;++j) c.fetch_add(1,std::memory_order_relaxed);} );
    for(auto &t:w) t.join();
    std::cout<<std::format("atomic  {:>2}T -> {:8.2f} ms\n",threads,dur(clk::now()-t0).count());
}
void test_mutex(unsigned threads,unsigned iters){
    int c=0; std::mutex m;
    auto t0=clk::now();
    std::vector<std::thread> w;
    for(unsigned i=0;i<threads;++i) w.emplace_back([&]{for(unsigned j=0;j<iters;++j){std::lock_guard<std::mutex> lg(m); ++c;}} );
    for(auto &t:w) t.join();
    std::cout<<std::format("mutex   {:>2}T -> {:8.2f} ms\n",threads,dur(clk::now()-t0).count());
}
int main(){
    const unsigned N=1'000'000;
    for(unsigned th: {1,2,4,8,16}){
        test_atomic(th,N);
        test_mutex(th,N);
        std::cout<<"--\n";
    }
}
