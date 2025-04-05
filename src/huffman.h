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
        Coder(dl, bits, ofname)
    {
        spdlog::info("Encoder Initialized");
    }
    template <typename T = uint8_t>
    void proc();
};

class Decoder: public Coder {
public:
    Decoder(DataLoader *dl, int bits, const char *ofname) :
        Coder(dl, bits, ofname)
    {
        spdlog::info("Decoder Initialized");
    }
    template <typename T = uint8_t>
    void proc();
};

#include "huffman.tpp"

#endif

