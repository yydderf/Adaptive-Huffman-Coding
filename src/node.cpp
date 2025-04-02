#include <cstdint>

#include "node.h"

Node::Node() : Node(0, NYT, nullptr) {}

Node::Node(uint64_t _id) : Node(_id, NYT, nullptr) {}

Node::Node(uint64_t _id, uint32_t _symbol) :
    Node(_id, _symbol, nullptr) {}

Node::Node(uint64_t _id, uint32_t _symbol, Node *_parent) :
    left(nullptr), right(nullptr), parent(_parent),
    id(_id), symbol(_symbol), weight(0) {}

void Node::_info(uint64_t *id, uint32_t *symbol, uint32_t *weight, Node **parent)
{
    *id = this->id;
    *symbol = this->symbol;
    *weight = this->weight;
    *parent = this->parent;
}
