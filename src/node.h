#ifndef _NODE_H
#define _NODE_H

#include <cstdint>
#include <boost/dynamic_bitset.hpp>

enum NODE_POS {
    NODE_LEFT,
    NODE_RIGHT,
};

class Node {
    friend class Tree;
    friend struct NodeAscComp;
public:
    Node();
    Node(uint64_t _id);
    Node(uint64_t _id, uint32_t _symbol);
    Node(uint32_t _symbol, Node *_parent, NODE_POS node_pos);
    Node(uint64_t _id, uint32_t _symbol, uint8_t _repr_size);
    Node(uint64_t _id, uint32_t _symbol, uint8_t _repr_size, Node *_parent);
    Node(uint64_t _id, uint32_t _symbol, uint8_t _repr_size, Node *_parent, bool mark_NYT);
    void _info(uint64_t *id, uint32_t *symbol, uint32_t *weight, Node **parent, bool *NYT);
    void set(uint32_t _symbol);
    bool external();
    void swap(Node *other);
private:
    Node *left;
    Node *right;
    Node *parent;
    // 2m - 1 for root node
    uint64_t id;
    bool NYT;
    // uncompressed symbol
    // symbol and NYT are mutually exclusive
    uint32_t symbol;
    // occurrences
    uint32_t weight;
    // compressed representation
    boost::dynamic_bitset<> repr;
};

#endif
