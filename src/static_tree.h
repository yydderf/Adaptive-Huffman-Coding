#ifndef _STATIC_TREE_H
#define _STATIC_TREE_H

#include "tree.h"
#include <vector>
#include <cstdint>

struct StaticNodeDscComp {
    bool operator()(Node* a, Node* b) const {
        return a->weight > b->weight;
    }
};

class StaticTree : public Tree {
public:
    StaticTree(uint32_t symbol_size) :
        Tree(symbol_size) {}
    // will call tree's destructor
    // no need to define another destructor
    // other double free will occur
    // ~StaticTree();
    void construct(const std::vector<uint32_t> &frequencies);
    void construct(const std::unordered_map<uint32_t, size_t> &frequencies);
    void _construct(std::priority_queue<Node*, std::vector<Node*>, StaticNodeDscComp> &pq);
};

#endif

