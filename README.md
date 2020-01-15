# llvm-block
llvm-block catches identical basic blocks between two modules.  
Since llvm-block draws on debug info, input modules should be generated with -g option.  

### Prerequisites
* cmake
* make
* LLVM 10
* clang 10

### Build
    git clone https://github.com/kc-ml2/llvm-block.git
    cd llvm-block && mkdir build && cd build
    cmake .. -DLLVM_ROOT=<path to llvm source root>
    make llvm-block
    ./llvm-block/llvm-block <before> <after>
    
### Quick Commands
    clang -O0 -g -Xclang -disable-O0-optnone -emit-llvm -S *.c
    llvm-link *.ll -S -o beforeg.ll
    opt -S <transform pass> -o afterg.ll

    <path to llvm-block>/llvm-block beforeg.ll afterg.ll 2> output

    opt —strip-debug -S beforeg.ll -o before.ll
    opt —strip-debug -S afterg.ll -o after.ll

    mkdir before after
    opt -dot-cfg before.ll
    mv .*.dot before
    opt -dot-cfg after.ll
    mv .*.dot after
