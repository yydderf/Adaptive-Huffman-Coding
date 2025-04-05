#!/usr/bin/env python3

import sys
from collections import Counter
import matplotlib.pyplot as plt

def count_byte_frequencies(file_path):
    # Open the file in binary mode and read its contents.
    with open(file_path, "rb") as f:
        data = f.read()
    # Count the frequencies of each byte.
    counter = Counter(data)
    # Create a list of frequencies for all byte values (0-255).
    frequencies = [counter.get(i, 0) for i in range(256)]
    return frequencies

def plot_frequencies(frequencies):
    plt.figure(figsize=(12, 6))
    plt.bar(range(256), frequencies, color='blue')
    plt.xlabel("Byte Value")
    plt.ylabel("Frequency")
    plt.title("Byte Frequency Histogram")
    plt.xticks(range(0, 256, 16))
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python plot_byte_frequencies.py <file_path>")
        sys.exit(1)
    file_path = sys.argv[1]
    frequencies = count_byte_frequencies(file_path)
    plot_frequencies(frequencies)

