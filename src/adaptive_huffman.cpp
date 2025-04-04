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

void Encoder::write_bits(const boost::dynamic_bitset<> &bits)
{
    boost::dynamic_bitset<> tmp_repr;
    std::string repr_buf;

    boost::to_string(bits, repr_buf);
    std::reverse(repr_buf.begin(), repr_buf.end());
    spdlog::info("Encoder write_bits input: {}", repr_buf);

    size_t old_size = this->bit_buf.size();
    this->bit_buf.resize(old_size + bits.size());
    for (size_t i = 0; i < bits.size(); ++i) {
        this->bit_buf[old_size + i] = bits[i];
    }
    while (this->bit_buf.size() >= 8) {
        uint8_t byte = 0;
        for (size_t i = 0; i < 8; ++i) {
            byte = (byte << 1) | (this->bit_buf[i] ? 1 : 0);
        }

        tmp_repr = this->bit_buf;
        tmp_repr.resize(8);
        boost::to_string(tmp_repr, repr_buf);
        std::reverse(repr_buf.begin(), repr_buf.end());
        spdlog::info("Encoder write_bits written: {}", repr_buf);

        this->ofs.write(reinterpret_cast<const char*>(&byte), 1);
        erase_left_bits(&(this->bit_buf), 8);
    }

    tmp_repr = this->bit_buf;
    boost::to_string(tmp_repr, repr_buf);
    std::reverse(repr_buf.begin(), repr_buf.end());
    spdlog::info("Encoder write_bits remaining: {}", repr_buf);
}

void Encoder::flush_bits()
{
    std::string repr_buf;
    uint8_t padding_size = static_cast<uint8_t>(this->bit_buf.size());
    if (this->bit_buf.size() > 0) {
        // size_t remaining = this->bit_buf.size();
        this->bit_buf.resize(8);
        uint8_t byte = 0;
        for (size_t i = 0; i < 8; ++i) {
            byte = (byte << 1) | (this->bit_buf[i] ? 1 : 0);
        }
        this->ofs.write(reinterpret_cast<const char*>(&byte), 1);
        boost::to_string(this->bit_buf, repr_buf);
        std::reverse(repr_buf.begin(), repr_buf.end());
        spdlog::info("Encoder write_bits remaining: {}", repr_buf);
        this->bit_buf.clear();
    }
    this->ofs.write(reinterpret_cast<const char*>(&padding_size), 1);
}

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
                boost::dynamic_bitset<> NYT_code = node_NYT->repr;
                this->write_bits(NYT_code);
                boost::dynamic_bitset<> literal = int_to_bitset(symbol, this->bits);
                this->write_bits(literal);
            } else {
                boost::dynamic_bitset<> code = curr_node->repr;
                this->write_bits(code);
            }
            tree.update(curr_node, symbol);
        }
        cnt += 1;
        std::cout << "[" << cnt * buf_size << "/" << this->dl->file_size << "]\r" << std::flush;
    }
    this->flush_bits();
    this->tree.display();
    this->ofs.flush();
    spdlog::warn("Data encode proc ended");
}

bool Decoder::read_bit(bool &bit)
{
    if (this->bit_buf.empty()) {
        uint8_t byte;
        if (!this->read_byte(byte)) {
            return false;
        }
        boost::dynamic_bitset<> tmp(8, static_cast<unsigned long long>(byte));
        for (size_t i = 0; i < 8; ++i) {
            this->bit_buf.push_back(tmp.test(7 - i));
        }
    }
    bit = this->bit_buf[0];
    // remove the first bit from the bitset
    // this->bit_buf.erase(this->bit_buf.begin());
    erase_left_bits(&(this->bit_buf), 1);
    return true;
}

boost::dynamic_bitset<> Decoder::read_bits(int n)
{
    boost::dynamic_bitset<> bits(n);
    for (int i = 0; i < n; ++i) {
        bool b;
        if (!this->read_bit(b)) {
            break;
        }
        bits[n - 1 - i] = b;
    }
    return bits;
}

bool Decoder::read_byte(uint8_t &byte)
{
    std::streamsize read_bytes;
    std::vector<char> *data = this->dl->get(1, &read_bytes);
    if (data == nullptr || read_bytes < 1) {
        return false;
    }
    byte = static_cast<uint8_t>((*data)[0]);
    return true;
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
        std::cout << "[" << acc_bits / 8 << "/" << tot_bytes << "]\r" << std::flush;
    }
proc_end:
    this->ofs.flush();
    spdlog::warn("Data decoding proc ended");
}
