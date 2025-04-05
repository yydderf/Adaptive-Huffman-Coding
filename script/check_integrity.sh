#!/bin/bash
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <binary> <target_file>"
    exit 1
fi

binary="$1"
binary_base=$(basename $binary)
input_file="$2"

base=$(basename "$input_file")
base="${base%.*}"
out_dir="out"
mkdir -p "$out_dir"
target="${out_dir}/${base}"
ext=""

if [ "$binary_base" == "huffman_main" ]; then
    ext=".h"
elif [ "$binary_base" == "adaptive_huffman_main" ]; then
    ext=".a"
else
    echo "Unknown binary: $binary"
    exit 1
fi

target_compressed="${target}.compressed${ext}"
target_original="${target}.original${ext}"

echo "Encoding file..."
./"$binary" -i "$input_file" -o "$target_compressed" -v w
if [ $? -ne 0 ]; then
    echo "Encoding failed."
    exit 1
fi

echo "Decoding file..."
./"$binary" -i "$target_compressed" -o "$target_original" -v w -d
if [ $? -ne 0 ]; then
    echo "Decoding failed."
    exit 1
fi

echo "Comparing original file with decoded file..."
diff <(sha256sum $input_file | cut -d ' ' -f 1) <(sha256sum $target_original | cut -d ' ' -f 1)
if [ $? -eq 0 ]; then
    echo "Integrity check passed: decoded file matches the original."
else
    echo "Integrity check failed: decoded file differs from the original."
fi

