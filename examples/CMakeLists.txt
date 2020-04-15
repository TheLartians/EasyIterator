cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# ---- Project ----

project(Examples CXX)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  NAME EasyIterator
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
)

# ---- Create binaries ----

file(GLOB examples ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach( example_source_file ${examples} )
  get_filename_component(filename ${example_source_file} NAME)
  string(REPLACE ".cpp" "" example_name ${filename} )
  add_executable(${example_name} ${example_source_file})
  target_link_libraries(${example_name} EasyIterator)            
  set_target_properties(${example_name} PROPERTIES CXX_STANDARD 17)        
endforeach()

