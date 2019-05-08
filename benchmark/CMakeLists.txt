cmake_minimum_required (VERSION 3.14)

# ---- create project ----

project(EasyIteratorBenchmark 
  LANGUAGES CXX
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CPM.cmake)

if (NOT TARGET EasyIterator)
  CPMAddPackage(
    NAME EasyIterator
    GIT_REPOSITORY https://github.com/TheLartians/EasyIterator.git
    GIT_TAG master
  )
endif()

CPMAddPackage(
  NAME googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  VERSION 1.4.1
  OPTIONS
   "BENCHMARK_ENABLE_TESTING Off"
   "BENCHMARK_USE_LIBCXX ON"
)

# patch google benchmark target
set_target_properties(benchmark PROPERTIES CXX_STANDARD 17)        

add_executable(EasyIteratorBenchmark "benchmark.cpp")
target_link_libraries(EasyIteratorBenchmark EasyIterator benchmark)
set_target_properties(EasyIteratorBenchmark PROPERTIES CXX_STANDARD 17)        
