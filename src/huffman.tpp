#ifndef _HUFFMAN_TPP
#define _HUFFMAN_TPP

#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstdint>
#include <boost/dynamic_bitset.hpp>
#include <spdlog/spdlog.h>

#include "argparse.h"
#include "dataloader.h"
#include "coder.h"
#include "huffman.h"
#include "utils.h"
#include "static_tree.h"

template <>
inline void Encoder::proc<uint8_t>()
{
    // Read the entire input from the DataLoader.
    auto raw_block = this->dl->next<uint8_t>();
    if (!raw_block) {
        spdlog::error("Failed to read input data");
        return;
    }
    
    // Build a frequency vector for 256 symbols (assume 8-bit symbols).
    spdlog::warn("Data encode proc started");
    std::vector<uint32_t> frequencies(256, 0);
    for (size_t i = 0; i < raw_block->size; i++) {
        frequencies[raw_block->raw_data[i]]++;
    }
    
    // Construct the static Huffman tree.
    StaticTree tree(256);
    tree.construct(frequencies);
    
    // Generate the code table by traversing the tree.
    // We'll map symbol (uint32_t) -> its Huffman code (string of '0' and '1').
    std::map<uint32_t, std::string> code_table;
    std::function<void(Node*, const std::string&)> traverse =
        [&](Node* node, const std::string& code) {
            // A leaf node: assume a leaf is one with no children.
            if (!node->left && !node->right) {
                code_table[node->symbol] = code;
                return;
            }
            if (node->left)
                traverse(node->left, code + "0");
            if (node->right)
                traverse(node->right, code + "1");
        };
    traverse(tree.get_root(), "");
    
    // Write header: write 256 frequency counts (each 4 bytes) so the decoder can reconstruct the tree.
    for (uint32_t f : frequencies) {
        ofs.write(reinterpret_cast<const char*>(&f), sizeof(f));
    }
    
    // Encode the input data.
    boost::dynamic_bitset<> bit_buffer;  // Local bit buffer to accumulate bits.
    
    for (size_t i = 0; i < raw_block->size; i++) {
        uint8_t symbol = raw_block->raw_data[i];
        const std::string &code = code_table[symbol];
        boost::dynamic_bitset<> bits(code.size());
        for (size_t j = 0; j < code.size(); ++j) {
            bits[j] = (code[j] == '1');
        }
        this->write_bits(bits);
    }
    
    // Flush any remaining bits, padding with zeros to complete a byte.
    this->flush_bits();
    ofs.flush();
    
    spdlog::warn("Data encode proc ended");
    // spdlog::info("Static Huffman encoding complete.");
}

template <>
inline void Encoder::proc<uint32_t>()
{
    // Read the entire input from the DataLoader.
    spdlog::warn("Data encode proc started");
    auto raw_block = this->dl->next<uint32_t>();
    if (!raw_block) {
        spdlog::error("Failed to read input data");
        return;
    }
    
    // Create frequency map from the 32-bit words.
    std::map<uint32_t, size_t> frequencies = raw_block->freq_map;
    
    // Write header:
    // First, write the number of unique symbols.
    uint32_t unique_count = frequencies.size();
    ofs.write(reinterpret_cast<const char*>(&unique_count), sizeof(unique_count));
    spdlog::info("unique count: {}", static_cast<int>(unique_count));
    
    // Then, write each unique symbol and its frequency.
    for (const auto &entry : frequencies) {
        uint32_t symbol = entry.first;
        uint32_t freq = entry.second;
        ofs.write(reinterpret_cast<const char*>(&symbol), sizeof(symbol));
        ofs.write(reinterpret_cast<const char*>(&freq), sizeof(freq));
    }
    
    // Construct the static Huffman tree using the frequency map.
    StaticTree tree(unique_count);
    tree.construct(frequencies);
    
    // Generate the code table by traversing the tree.
    std::map<uint32_t, std::string> code_table;
    std::function<void(Node*, const std::string&)> traverse =
        [&](Node* node, const std::string& code) {
            if (!node->left && !node->right) {
                code_table[node->symbol] = code;
                return;
            }
            if (node->left)
                traverse(node->left, code + "0");
            if (node->right)
                traverse(node->right, code + "1");
        };
    traverse(tree.get_root(), "");
    
    // Encode the input data (32-bit words).
    size_t coded_symbols = 0;
    for (size_t i = 0; i < raw_block->size; ++i) {
        uint32_t symbol = raw_block->raw_data[i];
        const std::string &code = code_table[symbol];
        // std::cout << "symbol: " << std::hex << symbol << ", code: " << code << std::endl;
        boost::dynamic_bitset<> bits(code.size());
        for (size_t j = 0; j < code.size(); ++j) {
            bits[j] = (code[j] == '1');
        }
        this->write_bits(bits);
        coded_symbols += 1;
    }
    
    // Flush any remaining bits.
    // this->flush_bits_32();
    this->flush_bits();
    coded_symbols += 1;
    std::cout << "coded symbols: " << coded_symbols << std::endl;
    ofs.flush();
    
    spdlog::warn("Data encode proc ended");
}

template <>
inline void Decoder::proc<uint8_t>()
{
    spdlog::warn("Data decoding proc started");
    const size_t num_symbols = 256;
    const size_t header_bytes = num_symbols * sizeof(uint32_t); // 1024 bytes
    std::vector<uint32_t> frequencies(num_symbols, 0);

    // 1. Read frequency header.
    for (size_t i = 0; i < num_symbols; i++) {
        uint32_t freq = 0;
        uint8_t b0, b1, b2, b3;
        if (!read_byte(b0) || !read_byte(b1) || !read_byte(b2) || !read_byte(b3)) {
            spdlog::error("Error reading frequency header at symbol index {}", i);
            return;
        }
        // Reconstruct frequency (assuming the same endianness as when written)
        freq = static_cast<uint32_t>(b0)
             | (static_cast<uint32_t>(b1) << 8)
             | (static_cast<uint32_t>(b2) << 16)
             | (static_cast<uint32_t>(b3) << 24);
        frequencies[i] = freq;
    }
    spdlog::info("Frequency header read successfully.");

    // 2. Build the static Huffman tree from the frequency table.
    StaticTree tree(num_symbols);
    tree.construct(frequencies);
    Node* root = tree.get_root();
    if (root == nullptr) {
        spdlog::error("Failed to construct Huffman tree from frequencies.");
        return;
    }

    // Step 3: Determine encoded data size.
    // Assume dl->file_size gives the total size of the file.
    size_t file_size = dl->file_size;
    if (file_size < header_bytes + 1) {
        spdlog::error("File too small to contain encoded data and padding.");
        return;
    }
    // The last byte in the file stores the padding.
    size_t encoded_data_bytes = file_size - header_bytes - 1;
    if (encoded_data_bytes == 0) {
        spdlog::error("No encoded data found");
        return;
    }
    size_t total_bits = encoded_data_bytes * 8;
    
    // Step 4: Read the encoded data bits.
    // read_bits(n) will read n bits using our internal bit_buf and read_bit().
    boost::dynamic_bitset<> encoded_bits = this->read_bits_rev(total_bits);
    
    // Step 5: Read the final padding byte.
    uint8_t padding;
    if (!read_byte(padding)) {
        spdlog::error("Failed to read padding byte.");
        return;
    }
    if (padding > 8) {
        spdlog::error("Invalid padding value: {}", padding);
        return;
    } else if (padding == 8) {
        padding = 0;
    }
    spdlog::warn("padding size: {}", padding);
    size_t valid_bits = total_bits - padding;
    spdlog::info("Encoded data: {} bytes, total bits: {}, valid bits: {}",
                 encoded_data_bytes, total_bits, valid_bits);

    // Step 6: Decode the bitstream.
    std::string repr_buf;
    boost::to_string(encoded_bits, repr_buf);
    spdlog::info("Decoder repr: {}", repr_buf);


    Node* current = root;
    for (size_t i = 0; i < valid_bits; ++i) {
        bool bit = encoded_bits[i];
        spdlog::info("bit: {}", bit);
        // Traverse the tree: go right for 1, left for 0.
        current = bit ? current->right : current->left;
        // current = bit ? current->left : current->right;
        if (current->external()) {
            // We've reached a leaf; output the symbol.
            boost::to_string(current->repr, repr_buf);
            spdlog::info("Decoder repr: {}", repr_buf);
            uint8_t symbol = static_cast<uint8_t>(current->symbol);
            ofs.write(reinterpret_cast<const char*>(&symbol), 1);
            current = root; // Restart for the next symbol.
        }
    }
    ofs.flush();
    spdlog::warn("Data decoding proc ended");
}

template <>
inline void Decoder::proc<uint32_t>()
{
    spdlog::warn("Data decoding proc started");
    // --- Step 1: Read header ---
    // First, read the unique count (number of unique symbols), 4 bytes.
    uint32_t unique_count = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t byte;
        if (!read_byte(byte)) {
            spdlog::error("Error reading header (unique_count).");
            return;
        }
        unique_count |= (static_cast<uint32_t>(byte) << (8 * i));
    }
    spdlog::info("Unique count: {}", static_cast<int>(unique_count));
    uint32_t header_size = 4;

    // Now, read unique_count entries (each: 4 bytes for symbol, 4 bytes for frequency).
    std::map<uint32_t, size_t> frequencies;
    for (uint32_t i = 0; i < unique_count; ++i) {
        uint32_t symbol = 0;
        uint32_t freq = 0;
        // Read symbol (4 bytes).
        for (int j = 0; j < 4; j++) {
            uint8_t byte;
            if (!read_byte(byte)) {
                spdlog::error("Error reading header (symbol) at entry {}.", i);
                return;
            }
            symbol |= (static_cast<uint32_t>(byte) << (8 * j));
        }
        // Read frequency (4 bytes).
        for (int j = 0; j < 4; j++) {
            uint8_t byte;
            if (!read_byte(byte)) {
                spdlog::error("Error reading header (frequency) at entry {}.", i);
                return;
            }
            freq |= (static_cast<uint32_t>(byte) << (8 * j));
        }
        header_size += 8;
        frequencies[symbol] = freq;
    }
    spdlog::info("Frequency table read successfully.");

    // --- Step 2: Reconstruct the Huffman tree ---
    StaticTree tree(unique_count);
    tree.construct(frequencies);

    Node* root = tree.get_root();
    if (root == nullptr) {
        spdlog::error("Failed to construct Huffman tree.");
        return;
    }

    // --- Step 3: Read encoded data ---
    // Assume dl->file_size holds the total file size.
    size_t file_size = dl->file_size;
    if (file_size < header_size + 1) {
        spdlog::error("File too small to contain encoded data and padding-validity byte.");
        return;
    }
    // The last byte is used for the number of valid bits in the final encoded byte.
    size_t encoded_data_bytes = file_size - header_size - 1;
    spdlog::info("filesize: {}, headersize: {}", file_size, header_size);
    
    std::vector<uint8_t> encoded_data;
    encoded_data.reserve(encoded_data_bytes);
    for (size_t i = 0; i < encoded_data_bytes; ++i) {
        uint8_t byte;
        if (!read_byte(byte)) {
            spdlog::error("Error reading encoded data byte at index {}.", i);
            return;
        }
        encoded_data.push_back(byte);
    }
    
    // Read final padding-validity byte.
    uint8_t padding_size = 0;
    if (!read_byte(padding_size)) {
        spdlog::error("Error reading padding-validity byte.");
        return;
    }
    if (padding_size > 8) { // padding_size is number of padded bits (0 to 31)
        spdlog::error("Invalid padding value: {}", padding_size);
        return;
    } else if (padding_size == 8) {
        padding_size = 0;
    }
    spdlog::warn("padding_size: {}", padding_size);
    // if (padding_size > 32) {
    //     spdlog::error("Invalid padding value: {}", padding_size);
    //     return;
    // } else if (padding_size == 32) {
    //     padding_size = 0;
    // }
    
    size_t total_bits = encoded_data_bytes * 8;
    size_t valid_bits = total_bits - padding_size;
    spdlog::warn("Encoded data: {} bytes, total bits: {}, valid bits: {}",
                 encoded_data_bytes, total_bits, valid_bits);

    // --- Step 5: Convert encoded data to dynamic bitset ---
    boost::dynamic_bitset<> encodedBits(valid_bits);
    size_t bit_index = 0;
    size_t total_units = encoded_data_bytes / 4;
    size_t unit;
    std::cout << "total units: " << total_units << std::endl;
    for (unit = 0; unit < total_units; unit++) {
        size_t offset = unit * 4;
        // Combine 4 bytes into a 32-bit word (assuming big-endian bit order within the word).
        uint32_t word = (static_cast<uint32_t>(encoded_data[offset]) << 24) |
                        (static_cast<uint32_t>(encoded_data[offset+1]) << 16) |
                        (static_cast<uint32_t>(encoded_data[offset+2]) << 8) |
                        (static_cast<uint32_t>(encoded_data[offset+3]));
        for (int bit = 31; bit >= 0; bit--) {
            bool b = ((word >> bit) & 1);
            encodedBits[bit_index++] = b;
        }
    }
    size_t offset = unit * 4;
    size_t tmp_tot = total_units * 32;
    int shl = 32;
    uint32_t word = 0;
    // std::cout << "offset: " << offset << ", encoded_data size: " << encoded_data.size() << std::endl;
    while (tmp_tot < total_bits) {
        shl -= 8;
        word |= (static_cast<uint32_t>(encoded_data[offset++]) << shl);
        tmp_tot += 8;
    }
    std::string code_str = "";
    for (int bit = 31; bit >= shl + padding_size; bit--) {
        bool b = ((word >> bit) & 1);
        code_str += (b ? '1' : '0');
        encodedBits[bit_index++] = b;
    }
    std::cout << code_str << std::endl;
    
    // --- Step 6: Decode the bitstream ---
    Node* current = root;
    size_t coded_symbols = 0;
    size_t i = 0;
    code_str = "";
    for (i = 0; i < valid_bits; i++) {
        bool bit = encodedBits[i];
        current = bit ? current->right : current->left;
        code_str += (bit ? '1' : '0');
        if (current->external()) {
            uint32_t symbol = current->symbol;
            // std::cout << "symbol: " << std::hex << symbol << ", code: " << code_str << std::endl;
            coded_symbols += 1;
            ofs.write(reinterpret_cast<const char*>(&symbol), sizeof(symbol));
            spdlog::info("Decoded symbol: {}", symbol);
            current = root;
            code_str = "";
        }
    }
    // std::cout << "coded_symbols: " << coded_symbols << std::endl;
    // std::cout << valid_bits << " " << encodedBits.size() << std::endl;
    ofs.flush();
    spdlog::warn("Data decoding proc ended (32-bit mode).");
}

#endif
