# Run Length Encoding (RLE) Compression

## Project Overview
This project implements a Run Length Encoding (RLE) compression algorithm in C. It supports both compression and decompression of text files, with a robust testing framework for validation.

## Functionality
- **Compression**: Converts repeated characters into character-count pairs
- **Decompression**: Restores original text from compressed format
- **Testing**: Automated test suite for both operations

## Technical Solution

### Algorithm
The implementation uses a single-pass approach for both compression and decompression:

#### Compression
- Reads input character by character
- Tracks consecutive character counts
- Outputs in format: "char count char count..."
- Time Complexity: O(n)
- Space Complexity: O(1)

#### Decompression
- Parses character-count pairs
- Expands each pair into original sequence
- Time Complexity: O(n)
- Space Complexity: O(1)

### Optimality
The solution is optimal because:
1. Single-pass processing
2. Constant extra space usage
3. Direct file I/O
4. No dynamic memory allocation
5. Minimal string operations

## Installation & Usage

### Prerequisites
- GCC Compiler
- Make (optional)
- Git

### Setup
```bash
git clone https://github.com/yourusername/rle-compression.git
cd rle-compression
gcc -o compress compression.c
