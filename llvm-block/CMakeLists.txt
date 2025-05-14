cmake_minimum_required(VERSION 3.13)


set(LLVM_LINK_COMPONENTS
  Core
  IRReader
  Support
  )

add_llvm_tool(llvm-block
  llvm-block.cpp
  table.cpp


  DEPENDS
  intrinsics_gen
  )

export_executable_symbols(llvm-block)
