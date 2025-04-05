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
    std::unordered_map<uint32_t, std::string> code_table;
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
    std::unordered_map<uint32_t, size_t> frequencies = raw_block->freq_map;
    
    // Write header:
    // First, write the number of unique symbols.
    uint32_t unique_count = frequencies.size();
    ofs.write(reinterpret_cast<const char*>(&unique_count), sizeof(unique_count));
    
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
    std::unordered_map<uint32_t, std::string> code_table;
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
    for (size_t i = 0; i < raw_block->size; i++) {
        uint32_t symbol = raw_block->raw_data[i];
        const std::string &code = code_table[symbol];
        boost::dynamic_bitset<> bits(code.size());
        for (size_t j = 0; j < code.size(); ++j) {
            bits[j] = (code[j] == '1');
        }
        spdlog::info("symbol: {}, code: {}", symbol, code);
        this->write_bits(bits);
    }
    
    // Flush any remaining bits.
    this->flush_bits();
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
    tree.display();
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
    uint32_t header_size = 4;
    spdlog::info("Unique count: {}", unique_count);

    // Now, read unique_count entries (each: 4 bytes for symbol, 4 bytes for frequency).
    std::unordered_map<uint32_t, size_t> frequencies;
    for (uint32_t i = 0; i < unique_count; i++) {
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
    
    std::vector<uint8_t> encoded_data;
    encoded_data.reserve(encoded_data_bytes);
    for (size_t i = 0; i < encoded_data_bytes; i++) {
        uint8_t byte;
        if (!read_byte(byte)) {
            spdlog::error("Error reading encoded data byte at index {}.", i);
            return;
        }
        encoded_data.push_back(byte);
    }
    
    // Read final padding-validity byte.
    uint8_t valid_last = 0;
    if (!read_byte(valid_last)) {
        spdlog::error("Error reading padding-validity byte.");
        return;
    }
    if (valid_last > 8) {
        spdlog::error("Invalid padding value: {}", valid_last);
        return;
    }
    
    // --- Step 4: Convert encoded data to bitset ---
    // All bytes except the last are fully valid.
    // size_t full_bytes = (encoded_data_bytes > 0) ? (encoded_data_bytes - 1) : 0;
    // size_t valid_bits = full_bytes * 8 + valid_last;
    size_t full_bytes;
    size_t valid_bits;
    if (valid_last == 0) {
        valid_bits = encoded_data_bytes * 8;
        full_bytes = encoded_data_bytes;
    } else {
        valid_bits = (encoded_data_bytes - 1) * 8 + valid_last;
        full_bytes = encoded_data_bytes - 1;
    }
    spdlog::info("Encoded data: {} bytes, valid bits: {}", encoded_data_bytes, valid_bits);
    
    boost::dynamic_bitset<> encodedBits(valid_bits);
    size_t bit_index = 0;
    // Process each full byte.
    for (size_t i = 0; i < full_bytes; i++) {
        for (int bit = 7; bit >= 0; bit--) {
            bool b = ((encoded_data[i] >> bit) & 1);
            encodedBits[bit_index++] = b;
        }
    }
    // Process the final (possibly partial) byte.
    if (encoded_data_bytes > 0) {
        uint8_t last_byte = encoded_data.back();
        for (int bit = 7; bit > 7 - static_cast<int>(valid_last); bit--) {
            bool b = ((last_byte >> bit) & 1);
            encodedBits[bit_index++] = b;
        }
    }
    
    // --- Step 5: Decode the bitstream ---
    Node* current = root;
    spdlog::info("valid_bits: {}", valid_bits);
    for (size_t i = 0; i < valid_bits; i++) {
        bool bit = encodedBits[i];
        current = bit ? current->right : current->left;
        if (current->external()) {
            // For 32-bit mode, write the symbol as 4 bytes.
            uint32_t symbol = current->symbol;
            ofs.write(reinterpret_cast<const char*>(&symbol), sizeof(symbol));
            spdlog::info("symbol: {}", symbol);
            current = root;
        }
    }
    ofs.flush();
    spdlog::warn("Data decoding proc ended");
}

#endif
