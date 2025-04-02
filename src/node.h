#ifndef _NODE_H
#define _NODE_H

#define NYT -1

#include <cstdint>

class Node {
    friend class Tree;
public:
    Node();
    Node(uint64_t _id);
    Node(uint64_t _id, uint32_t _symbol);
    Node(uint64_t _id, uint32_t _symbol, Node *parent);
    void _info(uint64_t *id, uint32_t *symbol, uint32_t *weight, Node **parent);
private:
    Node *left;
    Node *right;
    Node *parent;
    // 2m - 1 for root node
    uint64_t id;
    uint32_t symbol;
    uint32_t weight;
};

#endif
