#include <boost/dynamic_bitset.hpp>
#include <spdlog/spdlog.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "adaptive_huffman.h"
#include "utils.h"
#include "node.h"
#include "tree.h"

void Encoder::proc()
{
    std::streamsize buf_size, read_bytes;
    buf_size = 1024;
    uint32_t cnt = 0;
    spdlog::warn("Data encode proc started");
    while (!this->dl->eof()) {
        std::vector<char> *data = dl->get(buf_size, &read_bytes);
        spdlog::debug("Read {} bytes from file", read_bytes);
        if (data == nullptr) break;
        for (int i = 0; i < read_bytes; ++i) {
            uint32_t symbol = static_cast<uint32_t>((*data)[i]);
            Node *curr_node = tree.search(symbol);
            // first appearance
            if (curr_node == nullptr) {
                Node *node_NYT = this->tree.get_NYT();
                boost::dynamic_bitset<> NYT_code = this->tree.compute_code(node_NYT);
                this->write_bits(NYT_code);
                boost::dynamic_bitset<> literal = int_to_bitset(symbol, this->bits);
                this->write_bits(literal);
            } else {
                boost::dynamic_bitset<> code = this->tree.compute_code(curr_node);
                this->write_bits(code);
            }
            tree.update(curr_node, symbol);
        }
        cnt += 1;
        // std::cout << "[" << cnt * buf_size << "/" << this->dl->file_size << "]\r" << std::flush;
    }
    this->flush_bits();
    this->tree.display();
    this->ofs.flush();
    spdlog::warn("Data encode proc ended");
}

void Decoder::proc()
{
    boost::dynamic_bitset<> tmp_repr;
    std::string repr_buf;

    spdlog::warn("Data decoding proc started");
    uint64_t tot_bits = this->dl->file_size * 8 - static_cast<uint64_t>(this->dl->padding_size) - 8;
    uint64_t tot_bytes = tot_bits / 8;
    uint64_t acc_bits = 0;
    int read_bits_size = 8;
    while (true) {
        Node *curr_node = this->tree.get_root();
        if (curr_node == nullptr) break;

        // track path until external node is reached
        while (!curr_node->external()) {
            bool b;
            if (!this->read_bit(b) || acc_bits == tot_bits) {
                goto proc_end;
            }
            acc_bits += 1;
            curr_node = b ? curr_node->left : curr_node->right;
            if (curr_node == nullptr) {
                spdlog::error("Failed to decode: reached a null node");
                return;
            }
        }
        uint32_t symbol;
        if (curr_node->is_NYT()) {
            boost::dynamic_bitset<> literal = this->read_bits(read_bits_size);
            acc_bits += read_bits_size;
            symbol = bitset_to_int(literal);

            boost::to_string(literal, repr_buf);
            std::reverse(repr_buf.begin(), repr_buf.end());
            spdlog::info("Decoder write_bits literal: {}, symbol: {}", repr_buf, symbol);
        } else {
            symbol = curr_node->symbol;
            spdlog::info("Decoder write_bits symbol: {}", symbol);
        }
        char out_char = static_cast<char>(symbol);
        ofs.write(&out_char, 1);
        Node *node = this->tree.search(symbol);
        this->tree.update(node, symbol);
        // std::cout << "[" << acc_bits / 8 << "/" << tot_bytes << "]\r" << std::flush;
    }
proc_end:
    this->ofs.flush();
    spdlog::warn("Data decoding proc ended");
}
