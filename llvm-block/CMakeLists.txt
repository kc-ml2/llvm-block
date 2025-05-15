cmake_minimum_required(VERSION 3.12)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(llvm-block
  llvm-block.cpp
  table.cpp
)

llvm_map_components_to_libnames(llvm_libs
  Core
  IRReader
  Support
)

target_link_libraries(llvm-block ${llvm_libs})