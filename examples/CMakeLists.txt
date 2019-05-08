cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# ---- Project ----

project(Examples CXX)

# ---- CXX Flags ----

if(NOT CMAKE_CXX_STANDARD GREATER 17)
  set(CMAKE_CXX_STANDARD 17)
endif()

# ---- Requires ----

if(NOT TARGET EasyIterator)
  include(../cmake/CPM.cmake)
  CPMAddPackage(
    NAME EasyIterator
    GIT_REPOSITORY https://github.com/TheLartians/EasyIterator.git
    GIT_TAG master
  )
endif()

# ---- Create binaries ----

file(GLOB example_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach( example_source_file ${example_sources} )
  get_filename_component(filename ${example_source_file} NAME)
  string(REPLACE ".cpp" "" example_name ${filename} )
  add_executable(${example_name} ${example_source_file})
  target_link_libraries(${example_name} EasyIterator)            
endforeach()

