#!/bin/bash

set -e

# Check if $LLVM_BIN_PATH is set
if [ -z "$LLVM_BIN_PATH" ]; then
    echo -e "\033[31mLLVM_BIN_PATH is not set\033[0m"
    exit 1
fi

# Get LLVM version
LLVM_VERSION=$("$LLVM_BIN_PATH/llvm-config" --version | cut -d. -f1)
echo -e "\033[32mDetected LLVM version: $LLVM_VERSION\033[0m"

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
echo -e "\033[32mCompiling C files to LLVM IR...\033[0m"
if [ "$MACOS_FLAG" = true ]; then
    if [ "$LLVM_VERSION" -ge 16 ]; then
        "$LLVM_BIN_PATH/clang" -O$OPT_LEVEL -g -Xclang -disable-O0-optnone -emit-llvm -S *.c \
          -isysroot "$MACOS_SDK_PATH" -fno-discard-value-names
    else
        "$LLVM_BIN_PATH/clang" -O$OPT_LEVEL -g -Xclang -disable-O0-optnone -emit-llvm -S *.c \
          -isysroot "$MACOS_SDK_PATH"
    fi
else
    if [ "$LLVM_VERSION" -ge 16 ]; then
        "$LLVM_BIN_PATH/clang" -O$OPT_LEVEL -g -Xclang -disable-O0-optnone -emit-llvm -S *.c -fno-discard-value-names
    else
        "$LLVM_BIN_PATH/clang" -O$OPT_LEVEL -g -Xclang -disable-O0-optnone -emit-llvm -S *.c
    fi
fi

# Link LLVM IR files
echo -e "\033[32mLinking LLVM IR files...\033[0m"
if [ "$LLVM_VERSION" -ge 16 ]; then
    "$LLVM_BIN_PATH/llvm-link" *.ll -S -o beforeg.ll
else
    "$LLVM_BIN_PATH/llvm-link" *.ll -S -o beforeg.ll
fi

# Apply transformation pass
echo -e "\033[32mApplying transformation pass...\033[0m"
if [ "$LLVM_VERSION" -ge 16 ]; then
    "$LLVM_BIN_PATH/opt" beforeg.ll -S -passes=$LLVM_PASS -o afterg.ll
else
    "$LLVM_BIN_PATH/opt" beforeg.ll -S -$LLVM_PASS -o afterg.ll
fi

# Run llvm-block
echo -e "\033[32mRunning llvm-block...\033[0m"
"$LLVM_BLOCK_PATH/llvm-block" beforeg.ll afterg.ll 2> output

# Strip debug info
echo -e "\033[32mStripping debug info...\033[0m"
if [ "$LLVM_VERSION" -ge 16 ]; then
    "$LLVM_BIN_PATH/opt" --strip-debug -S beforeg.ll -o before.ll
    "$LLVM_BIN_PATH/opt" --strip-debug -S afterg.ll -o after.ll
else
    "$LLVM_BIN_PATH/opt" --strip-debug -S beforeg.ll -o before.ll
    "$LLVM_BIN_PATH/opt" --strip-debug -S afterg.ll -o after.ll
fi

# Generate CFGs
echo -e "\033[32mGenerating CFGs...\033[0m"
mkdir -p before after
if [ "$LLVM_VERSION" -ge 16 ]; then
    "$LLVM_BIN_PATH/opt" -passes=dot-cfg -f before.ll 2>&1 | grep -v "Writing" > /dev/null
    mv .*.dot before/
    "$LLVM_BIN_PATH/opt" -passes=dot-cfg -f after.ll 2>&1 | grep -v "Writing" > /dev/null
    mv .*.dot after/
else
    "$LLVM_BIN_PATH/opt" -dot-cfg -f before.ll 2>&1 | grep -v "Writing" > /dev/null
    mv .*.dot before/
    "$LLVM_BIN_PATH/opt" -dot-cfg -f after.ll 2>&1 | grep -v "Writing" > /dev/null
    mv .*.dot after/
fi