#ifndef _STATIC_TREE_H
#define _STATIC_TREE_H

#include "tree.h"
#include <vector>
#include <cstdint>

class StaticTree : public Tree {
public:
    StaticTree(uint32_t symbol_size) :
        Tree(symbol_size) {}
    // will call tree's destructor
    // no need to define another destructor
    // other double free will occur
    // ~StaticTree();
    void construct(const std::vector<uint32_t> &frequencies);
};

#endif

