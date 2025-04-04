#ifndef _TREE_H
#define _TREE_H

#include <cstdint>
#include <queue>
#include <boost/dynamic_bitset.hpp>
#include <set>

#include "node.h"

// TODO
// template Tree & Node so reduce the memory use for each node

struct NodeAscComp {
    bool operator()(const Node *lhs, const Node *rhs) const {
        return lhs->id < rhs->id;
    }
};

class Tree {
public:
    Tree();
    Tree(uint32_t _symbol_size);
    ~Tree();
    void update(Node *curr_node, uint32_t symbol);
    void _info(uint32_t *_symbol_size, Node **root);
    void _get_queue(Node *node, std::queue<std::pair<Node *, int>> *rq);
    void display();
    void display(Node *node);
    void _switch(Node *node);
    void _remove_from_set(Node *target_node);
    void swap(Node *node_a, Node *node_b);
    Node *search(uint32_t symbol);
    Node *get_NYT();
    Node *get_root();
protected:
    Node *root;
private:
    uint32_t symbol_size;
    uint32_t symbol_trans;
    std::unordered_map<uint32_t, Node*> symbol_map;
    std::unordered_map<uint32_t, std::set<Node*, NodeAscComp>> block_map;
    Node *node_NYT;
};

#endif
