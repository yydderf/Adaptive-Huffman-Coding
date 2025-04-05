#ifndef _HUFFMAN_H
#define _HUFFMAN_H

#include <boost/dynamic_bitset.hpp>
#include "coder.h"
#include "static_tree.h"
#include "dataloader.h"

// for huffman encoder and decoder
// tree is constructed in the proc
// for auto alloc and dealloc
class Encoder: public Coder {
public:
    Encoder(DataLoader *dl, int bits, const char *ofname) :
        Coder(dl, bits, ofname), tree(1 << bits)
    {
        spdlog::info("Encoder Initialized");
    }
    void proc() override;
};

class Decoder: public Coder {
public:
    Decoder(DataLoader *dl, int bits, const char *ofname) :
        Coder(dl, bits, ofname), tree(1 << bits)
    {
        spdlog::info("Decoder Initialized");
    }
    void proc() override;
};

#endif

