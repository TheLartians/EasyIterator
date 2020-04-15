cmake_minimum_required (VERSION 3.14)

option(EASY_ITERATOR_COMPARE_WITH_ITERTOOLS "benchmark itertools" OFF)

# ---- create project ----

project(EasyIteratorBenchmark 
  LANGUAGES CXX
)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  NAME EasyIterator
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
)

CPMAddPackage(
  NAME googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  VERSION 1.4.1
  OPTIONS
   "BENCHMARK_ENABLE_TESTING Off"
   "BENCHMARK_USE_LIBCXX ON"
)

if (googlebenchmark_ADDED)
  # patch google benchmark target
  set_target_properties(benchmark PROPERTIES CXX_STANDARD 17)
endif()

CPMAddPackage(
  NAME itertools
  GIT_REPOSITORY https://github.com/ryanhaining/cppitertools.git
  VERSION 1.0
  DOWNLOAD_ONLY Yes
)

if (googlebenchmark_ADDED)
  add_library(itertools INTERFACE)
  target_include_directories(itertools INTERFACE ${itertools_SOURCE_DIR})
endif()

# ---- Create standalone executable ----

add_executable(EasyIteratorBenchmark "benchmark.cpp")
set_target_properties(EasyIteratorBenchmark PROPERTIES CXX_STANDARD 17)        
target_link_libraries(EasyIteratorBenchmark benchmark itertools EasyIterator)
target_compile_definitions(EasyIteratorBenchmark PRIVATE "COMPARE_WITH_ITERTOOLS=1")


