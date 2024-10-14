cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(EasyIteratorTests
  LANGUAGES CXX
)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  NAME doctest
  GITHUB_REPOSITORY onqtam/doctest
  GIT_TAG v2.4.11
)

if (TEST_INSTALLED_VERSION)
  find_package(EasyIterator REQUIRED)
else()
  CPMAddPackage(
    NAME EasyIterator
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
  )
endif()

CPMAddPackage(
  NAME Format.cmake
  GITHUB_REPOSITORY TheLartians/Format.cmake
  VERSION 1.0
)

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(EasyIteratorTests ${sources})
target_link_libraries(EasyIteratorTests doctest EasyIterator)

set_target_properties(EasyIteratorTests PROPERTIES CXX_STANDARD 17)

# enable compiler warnings
if (NOT TEST_INSTALLED_VERSION)
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(EasyIterator INTERFACE -Wall -pedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(EasyIterator INTERFACE /W4 /WX)
  endif()
endif()

# ---- Add EasyIteratorTests ----

enable_testing() 

# Note: doctest and similar testing frameworks can automatically configure CMake tests
# For other testing frameworks add the tests target instead:
# add_test(EasyIteratorTests EasyIteratorTests)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(EasyIteratorTests)

# ---- code coverage ----

if (ENABLE_TEST_COVERAGE)
  target_compile_options(EasyIterator INTERFACE -O0 -g -fprofile-arcs -ftest-coverage --coverage)
  target_link_options(EasyIterator INTERFACE "--coverage")
endif()
