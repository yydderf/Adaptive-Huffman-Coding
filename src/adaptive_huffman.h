#ifndef _ADAPTIVE_HUFFMAN_H
#define _ADAPTIVE_HUFFMAN_H

#include <boost/dynamic_bitset.hpp>
#include <spdlog/spdlog.h>
#include "coder.h"
#include "tree.h"
#include "dataloader.h"

class Encoder: public Coder {
public:
    Encoder(DataLoader *dl, int bits, const char *ofname) :
        Coder(dl, bits, ofname), tree(1 << bits)
    {
        spdlog::info("Encoder Initialized");
    }
    void proc();
private:
    Tree tree;
    boost::dynamic_bitset<> bit_buf;
    void write_bits(const boost::dynamic_bitset<> &bits);
    void flush_bits();
};

class Decoder: public Coder {
public:
    Decoder(DataLoader *dl, int bits, const char *ofname) :
        Coder(dl, bits, ofname), tree(1 << bits)
    {
        spdlog::info("Decoder Initialized");
    }
    void proc();
private:
    Tree tree;
    boost::dynamic_bitset<> bit_buf;
    bool read_bit(bool &bit);
    boost::dynamic_bitset<> read_bits(int n);

    bool read_byte(uint8_t &byte);
};


#endif
