cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)

# ---- Dependencies ----

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  find_package(EasyIterator REQUIRED)
endif()

include(../cmake/CPM.cmake)

CPMAddPackage(
  NAME Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  VERSION 2.5.0
)

# ---- Create binary ----

file(GLOB EasyIteratorTests_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(EasyIteratorTests ${EasyIteratorTests_sources})
target_link_libraries(EasyIteratorTests Catch2 EasyIterator)
set_target_properties(EasyIteratorTests PROPERTIES CXX_STANDARD 17 COMPILE_FLAGS "-Wall -pedantic -Wextra -Werror")

# ---- Add EasyIteratorTests ----

ENABLE_TESTING() 
ADD_TEST(EasyIteratorTests EasyIteratorTests)

# ---- code coverage ----

if (${ENABLE_TEST_COVERAGE})
  set_target_properties(EasyIteratorTests PROPERTIES CXX_STANDARD 17 COMPILE_FLAGS "-O0 -g -fprofile-arcs -ftest-coverage --coverage")
  target_link_options(EasyIteratorTests PUBLIC "--coverage")
endif()
