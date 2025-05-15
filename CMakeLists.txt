cmake_minimum_required(VERSION 3.12)
project(llvm-block)

set(LLVM_ROOT "" CACHE PATH "Root of LLVM install.")

list(APPEND CMAKE_PREFIX_PATH
     "${LLVM_ROOT}/lib/cmake")

add_subdirectory(llvm-block)