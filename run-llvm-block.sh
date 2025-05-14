#!/bin/bash

# Check if $LLVM_BIN_PATH is set
if [ -z "$LLVM_BIN_PATH" ]; then
    echo "LLVM_BIN_PATH is not set"
    exit 1
fi

MACOS_FLAG=false
OPT_LEVEL=0
LLVM_PASS="mem2reg"  # Default pass

# Parse command line arguments

while [[ $# -gt 0 ]]; do
    case "$1" in
        --macos)
            MACOS_FLAG=true
            shift
            ;;
        --opt-level=*)
            OPT_LEVEL="${1#*=}"
            shift
            ;;
        --pass=*)
            LLVM_PASS="${1#*=}"
            shift
            ;;
        *)
            if [ -z "$TARGET_DIR" ]; then
                TARGET_DIR="$1"
            else
                echo "Error: Unexpected argument: $1"
                echo "Usage: $0 [--macos] [--opt-level=N] [--pass=<llvm-pass>] <target_directory>"
                exit 1
            fi
            shift
            ;;
    esac
done

# Check if target directory is provided
if [ -z "$TARGET_DIR" ]; then
    echo "Usage: $0 [--macos] [--opt-level=<opt-level>] [--pass=<llvm-pass>] <target_directory>"
    exit 1
fi

LLVM_BLOCK_PATH="$(pwd)/build/llvm-block"

# Get macOS SDK path
if [ "$MACOS_FLAG" = true ]; then
    MACOS_SDK_PATH=$(xcrun --sdk macosx --show-sdk-path)
else
    MACOS_SDK_PATH=""
fi

# Change to target directory
echo "Changing to target directory... $TARGET_DIR"
cd "$TARGET_DIR"

# Compile C files to LLVM IR
echo "Compiling C files to LLVM IR..."

if [ "$MACOS_FLAG" = true ]; then
    "$LLVM_BIN_PATH/clang" -O$OPT_LEVEL -g -Xclang -disable-O0-optnone -emit-llvm -S *.c \
      -isysroot "$MACOS_SDK_PATH"
else
    "$LLVM_BIN_PATH/clang" -O$OPT_LEVEL -g -Xclang -disable-O0-optnone -emit-llvm -S *.c
fi

# Link LLVM IR files
"$LLVM_BIN_PATH/llvm-link" *.ll -S -o beforeg.ll

# Apply transformation pass
"$LLVM_BIN_PATH/opt" beforeg.ll -S -$LLVM_PASS -o afterg.ll

# Run llvm-block
"$LLVM_BLOCK_PATH/llvm-block" beforeg.ll afterg.ll 2> output

# Strip debug info
"$LLVM_BIN_PATH/opt" --strip-debug -S beforeg.ll -o before.ll
"$LLVM_BIN_PATH/opt" --strip-debug -S afterg.ll -o after.ll

# print log
echo "Generating CFGs..."

# Generate CFGs
mkdir -p before after
"$LLVM_BIN_PATH/opt" -dot-cfg -f before.ll
mv .*.dot before/
"$LLVM_BIN_PATH/opt" -dot-cfg -f after.ll
mv .*.dot after/