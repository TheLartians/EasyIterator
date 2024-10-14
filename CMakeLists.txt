cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----

project(
  EasyIterator
  VERSION 1.5
  LANGUAGES CXX)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

include(cmake/CPM.cmake)

# PackageProject.cmake will be used to make our target installable
cpmaddpackage(NAME PackageProject.cmake GITHUB_REPOSITORY
              TheLartians/PackageProject.cmake VERSION 1.0)

# ---- Header target ----

file(GLOB_RECURSE headers CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
add_library(EasyIterator-headers EXCLUDE_FROM_ALL ${headers})
set_target_properties(EasyIterator-headers PROPERTIES LINKER_LANGUAGE CXX)

# ---- Create library ----

add_library(EasyIterator INTERFACE)

target_compile_options(EasyIterator INTERFACE "$<$<BOOL:${MSVC}>:/permissive->")
set_target_properties(EasyIterator PROPERTIES INTERFACE_COMPILE_FEATURES
                                              cxx_std_17)

target_include_directories(
  EasyIterator
  INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>)

# ---- Create an installable target ----

packageproject(
  NAME
  ${PROJECT_NAME}
  VERSION
  ${PROJECT_VERSION}
  BINARY_DIR
  ${PROJECT_BINARY_DIR}
  INCLUDE_DIR
  ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION
  include/${PROJECT_NAME}-${PROJECT_VERSION})
