# llvm-block
llvm-block catches identical basic blocks between two modules.  
Since llvm-block draws on debug info, input modules should be generated with -g option.  

### Prerequisites
```
cmake >= 3.25 
llvm >= 14
```

### Build
```bash
git clone https://github.com/kc-ml2/llvm-block.git
cd llvm-block
export LLVM_ROOT=<path to llvm source root>
cmake --preset=default && cmake --build build 
./llvm-block/llvm-block <before> <after>
```
*If would like to build in a MacOS environment, run with the `macos` preset instead (`cmake --preset=macos && cmake --build build`)

### Quick Commands
    clang -O0 -g -Xclang -disable-O0-optnone -emit-llvm -S *.c
    llvm-link *.ll -S -o beforeg.ll
    opt beforeg.ll -S <transform pass> -o afterg.ll

    <path to llvm-block>/llvm-block beforeg.ll afterg.ll 2> output

    opt —strip-debug -S beforeg.ll -o before.ll
    opt —strip-debug -S afterg.ll -o after.ll

    mkdir before after
    opt -dot-cfg before.ll
    mv .*.dot before
    opt -dot-cfg after.ll
    mv .*.dot after

### TLDR script
If you are too bothered to run the commands above, simply run `run-llvm-block.sh` with the following instructions.

1. First add a `*.c` file to any directory, say you have `foo/a.c`.

2. Then simply run `bash run-llvm-block.sh foo` (checkout the code for more options if you are on MacOS).
