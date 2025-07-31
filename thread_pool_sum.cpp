
// =========================================================
//  * Minimal lock‑free MPMC thread‑pool
//  * Re‑uses workers across many tasks to amortise overhead
// =========================================================
#include <bits/stdc++.h>
#include <thread>
#include <queue>
#include <functional>
#include <atomic>
#include <condition_variable>

class ThreadPool {
    struct task_wrapper {
        std::function<void()> fn;
    };
    std::vector<std::thread> workers;
    std::queue<task_wrapper>                  q;
    std::mutex                               m;
    std::condition_variable                  cv;
    std::atomic<bool>                        stop=false;
public:
    explicit ThreadPool(unsigned n = std::thread::hardware_concurrency()) {
        for (unsigned i=0;i<n;++i) {
            workers.emplace_back([this]{ this->worker(); });
        }
    }
    ~ThreadPool(){
        stop=true; cv.notify_all();
        for(auto &w:workers) w.join();
    }
    template<typename F>
    void enqueue(F&& f){
        {
            std::lock_guard<std::mutex> lg(m);
            q.push({std::forward<F>(f)});
        }
        cv.notify_one();
    }
private:
    void worker(){
        while(true){
            task_wrapper task;
            {
                std::unique_lock<std::mutex> ul(m);
                cv.wait(ul,[this]{return stop||!q.empty();});
                if(stop && q.empty()) return;
                task = std::move(q.front()); q.pop();
            }
            task.fn();
        }
    }
};

std::uint64_t pool_sum(const std::vector<int>& v, ThreadPool &pool, unsigned n_tasks){
    const std::size_t n = v.size();
    std::vector<std::uint64_t> partial(n_tasks);
    std::barrier sync(n_tasks+1);
    const std::size_t chunk=(n+n_tasks-1)/n_tasks;
    for(unsigned t=0;t<n_tasks;++t){
        const std::size_t start=t*chunk;
        const std::size_t stop=std::min(start+chunk,n);
        pool.enqueue([&,t,start,stop]{
            partial[t]=std::accumulate(v.begin()+start,v.begin()+stop,0ull);
            sync.arrive_and_wait();
        });
    }
    sync.arrive_and_wait(); // main thread waits
    return std::accumulate(partial.begin(),partial.end(),0ull);
}

int main(){
    const std::size_t N=50'000'000;
    auto data=std::vector<int>(N,1);
    ThreadPool pool;
    for(unsigned tasks: {1,2,4,8,16,32}){
        auto t0=clk::now();
        auto s=pool_sum(data,pool,tasks);
        auto dt=dur(clk::now()-t0).count();
        std::cout<<std::format("tasks {:>2}: sum {} -> {:>8.2f} ms\n",tasks,s,dt);
    }
}