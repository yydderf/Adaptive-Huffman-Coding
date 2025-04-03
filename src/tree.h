#ifndef _TREE_H
#define _TREE_H

#include <cstdint>
#include <queue>
#include <boost/dynamic_bitset.hpp>

#include "node.h"

// TODO
// template Tree & Node so reduce the memory use for each node
class Tree {
public:
    Tree();
    Tree(uint32_t _symbol_size);
    ~Tree();
    void update(uint32_t symbol);
    void _info(uint32_t *_symbol_size, Node **root);
    std::queue<std::pair<Node*, int>> _get_queue(Node *node);
    void display();
    void display(Node *node);
    void _switch(Node *node);
    Node *search(uint32_t symbol);
private:
    uint32_t symbol_size;
    uint32_t symbol_trans;
    std::unordered_map<uint32_t, Node*> symbol_map;
    std::unordered_map<uint32_t, std::vector<Node*>> block_map;
    boost::dynamic_bitset<> NYT;
    Node *root;
    Node *node_NYT;
};

#endif
