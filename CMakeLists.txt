cmake_minimum_required(VERSION 3.20)
project(ParallelismConcurrency CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On *nix we need pthread
if (UNIX AND NOT APPLE)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
endif()

# ------------------------------------------------------------
# List of example sources: add new files here
set(EXAMPLES
  multithread_sum.cpp
  thread_pool_sum.cpp
  async_sum.cpp
  deadlock_demo.cpp
  race_condition.cpp
  priority_inversion.cpp
  atomic_vs_mutex.cpp
)

foreach(src ${EXAMPLES})
  get_filename_component(target ${src} NAME_WE)
  add_executable(${target} ${src})
  if (UNIX AND NOT APPLE)
    target_link_libraries(${target} PRIVATE Threads::Threads)
  elseif (APPLE)
    target_link_libraries(${target} PRIVATE pthread)
  endif()
endforeach()

# ------------------------------------------------------------
# Optional: enable `ctest` smoke tests (runs executables)
enable_testing()
foreach(src ${EXAMPLES})
  get_filename_component(target ${src} NAME_WE)
  add_test(NAME run_${target} COMMAND ${target})
endforeach()
