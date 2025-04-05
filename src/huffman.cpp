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

void Encoder::proc() {
    // Read the entire input from the DataLoader.
    auto raw_block = dl->next();
    if (!raw_block) {
        spdlog::error("Failed to read input data");
        return;
    }
    
    // Build a frequency vector for 256 symbols (assume 8-bit symbols).
    spdlog::warn("Data encode proc started");
    std::vector<uint32_t> frequencies(256, 0);
    for (size_t i = 0; i < raw_block->size; i++) {
        frequencies[raw_block->raw_bytes[i]]++;
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
        uint8_t symbol = raw_block->raw_bytes[i];
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

void Decoder::proc() {
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
    size_t total_bits = encoded_data_bytes * 8;
    
    // Step 4: Read the encoded data bits.
    // read_bits(n) will read n bits using our internal bit_buf and read_bit().
    boost::dynamic_bitset<> encoded_bits = read_bits(total_bits, 1);
    
    // Step 5: Read the final padding byte.
    uint8_t padding;
    if (!read_byte(padding)) {
        spdlog::error("Failed to read padding byte.");
        return;
    }
    if (padding > 7) {
        spdlog::error("Invalid padding value: {}", padding);
        return;
    }
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

