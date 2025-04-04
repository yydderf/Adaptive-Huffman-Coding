#ifndef _CODER_H
#define _CODER_H

#include <boost/dynamic_bitset.hpp>
#include <spdlog/spdlog.h>
#include "dataloader.h"
#include <fstream>

class Coder {
public:
    Coder(DataLoader *dl, int bits, const char *ofname);
    virtual void proc() { spdlog::warn("Proc Not Implemented"); };
protected:
    DataLoader *dl;
    std::ofstream ofs;
    int bits;

    boost::dynamic_bitset<> bit_buf;
    void write_bits(const boost::dynamic_bitset<> &bits);
    void flush_bits();

    bool read_bit(bool &bit);
    boost::dynamic_bitset<> read_bits(int n, bool rev);
    bool read_byte(uint8_t &byte);
};

#endif
