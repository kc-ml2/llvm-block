cmake_minimum_required(VERSION 3.12)
project(llvm-block)

set(LLVM_ROOT "" CACHE PATH "Root of LLVM install.")

list(APPEND CMAKE_PREFIX_PATH
     "${LLVM_ROOT}/lib/cmake")

message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

# Find LLVM package
find_package(LLVM REQUIRED CONFIG) # This is required for llvm version check
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

# Get LLVM version components
set(LLVM_VERSION_MAJOR ${LLVM_VERSION_MAJOR})

message(STATUS "LLVM Version Major: ${LLVM_VERSION_MAJOR}")

if (LLVM_VERSION_MAJOR GREATER_EQUAL 16)
    message(STATUS "Using C++17 standard (LLVM >= 16)")
    set(CMAKE_CXX_STANDARD 17)
else()
    message(STATUS "Using C++14 standard (LLVM < 16)")
    set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(llvm-block)