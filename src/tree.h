#ifndef _TREE_H
#define _TREE_H

#include <cstdint>
#include <queue>

#include "node.h"

class Tree {
public:
    Tree();
    Tree(uint32_t _symbol_size);
    int get_node(char symbol);
    void update(char symbol);
    void _info(uint32_t *_symbol_size, Node **root);
    std::queue<std::pair<Node*, int>> _get_queue();
    void display();
private:
    uint32_t symbol_size;
    Node *root;
};

#endif
