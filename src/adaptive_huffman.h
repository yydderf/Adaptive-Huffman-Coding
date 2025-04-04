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
    void proc() override;
private:
    Tree tree;
};

class Decoder: public Coder {
public:
    Decoder(DataLoader *dl, int bits, const char *ofname) :
        Coder(dl, bits, ofname), tree(1 << bits)
    {
        spdlog::info("Decoder Initialized");
    }
    void proc() override;
private:
    Tree tree;
};


#endif
