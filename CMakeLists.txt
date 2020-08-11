cmake_minimum_required(VERSION 3.10)

project(SortVisualizer LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

include(cmake/CompilerWarnings.cmake)
include(cmake/Coverage.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/StandardProjectSettings.cmake)

add_library(project_options INTERFACE)
add_library(project::options ALIAS project_options)
target_compile_features(project_options INTERFACE cxx_std_17)

add_library(project_warnings INTERFACE)
add_library(project::warnings ALIAS project_warnings)
set_project_warnings(project_warnings)

enable_sanitizers(project_options)
enable_coverage(project_options)

find_package(spdlog REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glad REQUIRED)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/)

option(BUILD_TESTS "Build the tests for the sorting algorithms" OFF)

if(BUILD_TESTS)
  find_package(doctest REQUIRED)

  enable_testing()
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests/)
endif()
