cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# ---- Project ----

project(EasyIterator 
  VERSION 1.1
  LANGUAGES CXX
)

# ---- Configuration variables ----

option(EASY_ITERATOR_ENABLE_TESTS "Build tests" OFF)
option(EASY_ITERATOR_BUILD_EXAMPLES "Enable examples" OFF)
option(EASY_ITERATOR_BUILD_BENCHMARK "Enable benchmark" OFF)

# ---- Include guards ----

if(TARGET EasyIterator)
  return()
endif()

if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# ---- Header target ----

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
add_library(EasyIterator-headers EXCLUDE_FROM_ALL ${headers})
SET_TARGET_PROPERTIES(EasyIterator-headers PROPERTIES LINKER_LANGUAGE CXX)

# ---- Create library ----

add_library(EasyIterator INTERFACE)

target_include_directories(EasyIterator
  INTERFACE 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/EasyIteratorConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

# ---- Install ----

install(
  TARGETS EasyIterator
  EXPORT EasyIteratorTargets
  LIBRARY DESTINATION lib COMPONENT Runtime
  ARCHIVE DESTINATION lib COMPONENT Development
  RUNTIME DESTINATION bin COMPONENT Runtime
  PUBLIC_HEADER DESTINATION include COMPONENT Development
  BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/EasyIteratorConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/EasyIteratorConfig.cmake"
  INSTALL_DESTINATION lib/cmake/EasyIterator
)

install(
  EXPORT EasyIteratorTargets 
  DESTINATION lib/cmake/EasyIterator
)

install(
  FILES 
    "${PROJECT_BINARY_DIR}/EasyIteratorConfigVersion.cmake"
    "${PROJECT_BINARY_DIR}/EasyIteratorConfig.cmake"
  DESTINATION 
    lib/cmake/EasyIterator
)

install(
  DIRECTORY ${PROJECT_SOURCE_DIR}/include/
  DESTINATION include
)

# ---- Test ----

if(${EASY_ITERATOR_ENABLE_TESTS})
  ENABLE_TESTING()  
  add_subdirectory(tests)  
endif()

# ---- Examples ----

if(${EASY_ITERATOR_BUILD_EXAMPLES})
  add_subdirectory(examples)  
endif()

# ---- Benchmark ----

if(${EASY_ITERATOR_BUILD_BENCHMARK})
  add_subdirectory(benchmark)  
endif()

