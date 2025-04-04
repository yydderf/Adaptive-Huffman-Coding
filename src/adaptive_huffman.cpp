#include <boost/dynamic_bitset.hpp>
#include <spdlog/spdlog.h>
#include <iostream>
#include <sstream>
#include <vector>

#include "argparse.h"
#include "node.h"
#include "tree.h"
#include "utils.h"
#include "adaptive_huffman.h"

void Encoder::write_bits(const boost::dynamic_bitset<> &bits)
{
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
        this->ofs.write(reinterpret_cast<const char*>(&byte), 1);
        boost::dynamic_bitset<> new_buf(this->bit_buf.size() - 8);
        for (size_t i = 8; i < this->bit_buf.size(); ++i) {
            new_buf[i - 8] = this->bit_buf[i];
        }
        this->bit_buf = new_buf;
    }
}

void Encoder::flush_bits()
{
    if (this->bit_buf.size() > 0) {
        // size_t remaining = this->bit_buf.size();
        this->bit_buf.resize(8);
        uint8_t byte = 0;
        for (size_t i = 0; i < 8; ++i) {
            byte = (byte << 1) | (this->bit_buf[i] ? 1 : 0);
        }
        this->ofs.write(reinterpret_cast<const char*>(&byte), 1);
        this->bit_buf.clear();
    }
}

void Encoder::proc()
{
    std::streamsize buf_size, read_bytes;
    buf_size = 1024;
    uint32_t cnt = 0;
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
    spdlog::info("Data encode proc ends");
}

int main(int argc, char **argv)
{
    auto args = Arguments(argc, argv);

    spdlog::info("adaptive huffman init");
    // data loader
    DataLoader dl(args.ifname, args.bits, READ_ONCE | MODE_ENC);
    if (!dl.ok()) {
        return 1;
    }
    // coder
    Encoder encoder(&dl, args.bits, args.ofname);
    encoder.proc();
    // if (!encoder.proc()) {
    //     return 1;
    // }

    return 0;
}

