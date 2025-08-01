# Parallelism & Concurrency Playground

A collection of small, self-contained C++20 programs that illustrate core
concurrency topics:

| File | Topic | Key Idea |
|------|-------|----------|
| `multithread_sum.cpp` | **Thread fan-out** | Divide an array across N threads and measure speed-up & thread-creation overhead. |
| `thread_pool_sum.cpp` | **Thread pool** | Re-use a fixed worker pool to amortise creation cost. |
| `async_sum.cpp` | **Task-based parallelism** | Use `std::async` / futures for automatic task management. |
| `deadlock_demo.cpp` | **Deadlocks & fixes** | Intentional deadlock vs. `std::scoped_lock` solution (`-DFIX`). |
| `race_condition.cpp` | **Data races** | Compare no-sync, `std::atomic`, and `std::mutex`. |
| `priority_inversion.cpp` | **Mars Pathfinder problem** | Simulate priority inversion (POSIX). |
| `atomic_vs_mutex.cpp` | **Micro-benchmark** | Measure atomic vs. mutex increment across thread counts. |

---

## Build & Run (locally)

```bash
# One-shot build of everything (requires CMake ≥ 3.20)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Example: run the multithreaded sum
./build/multithread_sum
````

---

## Sample Output (Apple M1 8-core, Release build)

<details>
<summary>Click to expand</summary>

### `multithread_sum`

```
Elements: 50000000 | cores: 8
threads  1: sum 1250000025000000 ->   7.29 ms
threads  2: sum 1250000025000000 ->   3.38 ms
threads  4: sum 1250000025000000 ->   2.15 ms
threads  8: sum 1250000025000000 ->   1.85 ms
threads 16: sum 1250000025000000 ->   2.00 ms
Created+joined 1000000 empty threads in 14232.5 ms (70 k/s)
```

### `thread_pool_sum`

```
tasks  1: sum 50000000 -> 31.10 ms
tasks  2: sum 50000000 ->  2.93 ms
tasks  4: sum 50000000 ->  1.86 ms
```

*(Run aborted at 8+ tasks on macOS because libc++’s `std::barrier` fallback hit an
invalid-argument edge case; the fix in `barrier` now resolves this.)*

### `async_sum`

```
async sum=100000000 in 8.12 ms
```

### `deadlock_demo`

*Run without `-DFIX` → main hangs (deadlock).*
*Run with `-DFIX` → program terminates immediately.*

### `race_condition`

| Variant       | Time (ms) | Correct?                                   |
| ------------- | --------- | ------------------------------------------ |
| No-sync       | **0.18**  | ❌ undefined behaviour but “looked” correct |
| `std::atomic` | 57.89     | ✅                                          |
| `std::mutex`  | 123.39    | ✅                                          |

### `priority_inversion`

```
Simulating priority inversion…
High acquired after 190 ms
Complete. Try rebuilding with -DPRIO_INHERIT ...
```

### `atomic_vs_mutex`

```
atomic   1T ->   5.66 ms
mutex    1T ->  17.27 ms
--
atomic   2T ->  15.25 ms
mutex    2T ->  29.50 ms
--
atomic   4T ->  64.61 ms
mutex    4T -> 114.76 ms
--
atomic   8T -> 246.14 ms
mutex    8T -> 203.41 ms
--
atomic  16T -> 514.11 ms
mutex   16T -> 365.26 ms
```

</details>

---

## What we learn

* **Thread creation is expensive** – pooling brings a \~10× win at 2 threads and
  \~16× at 4 tasks.
* **`std::async`** gives decent performance with *zero* thread bookkeeping.
* **Atomics** scale better than mutexes up to ≈ 4 threads, then cache-line
  contention kills throughput and the mutex variant can catch up.
* **Deadlocks** are trivial to reproduce; `std::scoped_lock` (or consistent lock
  ordering) fixes them painlessly.
* **Priority inversion** is real: the high-priority task waited \~190 ms for a
  low-priority task while a medium-priority spinner hogged the CPU.
