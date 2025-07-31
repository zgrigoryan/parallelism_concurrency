
// =========================================================
//  * std::async / future based parallel reduction
//  * Illustrates task‑based decomposition simplicity
// =========================================================
#include <bits/stdc++.h>
#include <future>

std::uint64_t async_sum(const int* first,const int* last,unsigned depth=0){
    const std::size_t n = last-first;
    if(n<1'000'000 || depth>2) return std::accumulate(first,last,0ull);
    const int* mid = first + n/2;
    auto f = std::async(std::launch::async, async_sum, first, mid, depth+1);
    auto right = async_sum(mid,last,depth+1);
    return f.get()+right;
}
int main(){
    std::vector<int> v(100'000'000,1);
    auto t0=clk::now();
    auto s=async_sum(v.data(),v.data()+v.size());
    auto dt=dur(clk::now()-t0).count();
    std::cout<<"async sum="<<s<<" in "<<dt<<" ms\n";
}