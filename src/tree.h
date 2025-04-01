#ifndef _TREE_H
#define _TREE_H

#include <cstdint>

class Node {
public:
    Node();
private:
    Node *left;
    Node *right;
    Node *parent;
    uint32_t weight;
};

class Tree {
public:
    Tree();
    int get_node(unsigned char symbol);
private:
    Node *root;
};

#endif
