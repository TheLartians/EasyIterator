cmake_minimum_required (VERSION 3.14)

option(EASY_ITERATOR_COMPARE_WITH_ITERTOOLS "benchmark itertools" OFF)

# ---- create project ----

project(EasyIteratorBenchmark 
  LANGUAGES CXX
)

add_executable(EasyIteratorBenchmark "benchmark.cpp")
set_target_properties(EasyIteratorBenchmark PROPERTIES CXX_STANDARD 17)        

# ---- Dependencies ----

include(../cmake/CPM.cmake)

if (NOT TARGET EasyIterator)
  CPMAddPackage(
    NAME EasyIterator
    GIT_REPOSITORY https://github.com/TheLartians/EasyIterator.git
    GIT_TAG master
  )
endif()

target_link_libraries(EasyIteratorBenchmark EasyIterator)


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
target_link_libraries(EasyIteratorBenchmark benchmark)

if (${EASY_ITERATOR_COMPARE_WITH_ITERTOOLS})
  CPMAddPackage(
    NAME itertools
    GIT_REPOSITORY https://github.com/ryanhaining/cppitertools.git
    VERSION 1.0
    DOWNLOAD_ONLY Yes
  )

  CPMGetProperties(itertools)
  add_library(itertools INTERFACE)
  target_include_directories(itertools INTERFACE ${itertools_SOURCE_DIR})
  target_compile_definitions(itertools INTERFACE "COMPARE_WITH_ITERTOOLS=1")

  target_link_libraries(EasyIteratorBenchmark itertools)
endif()

