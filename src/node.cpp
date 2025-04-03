#include <cstdint>

#include <spdlog/spdlog.h>

#include "node.h"

Node::Node() :
    Node(0, 0, 0, nullptr, true) {}

Node::Node(uint64_t _id) :
    Node(_id, 0, 0, nullptr, true) {}

Node::Node(uint32_t _symbol, Node *_parent, NODE_POS node_pos) :
    left(nullptr), right(nullptr), parent(_parent),
    symbol(_symbol), weight(0)
{
    // left - NYT, right - symbol
    // copy from old NYT's repr
    // right - append 0
    // left  - append 1
    size_t repr_size = _parent->repr.size() + 1;

    this->repr = _parent->repr;
    this->repr.resize(repr_size);

    if (node_pos == NODE_LEFT) {
        this->id = _parent->id - 2;
        this->NYT = true;
        this->repr[repr_size - 1] = 1;
        _parent->left = this;
        _parent->NYT = false;
    } else {
        this->id = _parent->id - 1;
        this->NYT = false;
        this->repr[repr_size - 1] = 0;
        _parent->right = this;
    }
}

Node::Node(uint64_t _id, uint32_t _symbol) :
    Node(_id, _symbol, 1, nullptr, false) {}

Node::Node(uint64_t _id, uint32_t _symbol, uint8_t _repr_size) :
    Node(_id, _symbol, _repr_size, nullptr, false) {}

Node::Node(uint64_t _id, uint32_t _symbol, uint8_t _repr_size, Node *_parent) :
    Node(_id, _symbol, _repr_size, _parent, false) {}

Node::Node(uint64_t _id, uint32_t _symbol, uint8_t _repr_size, Node *_parent, bool mark_NYT) :
    left(nullptr), right(nullptr), parent(_parent),
    id(_id), NYT(mark_NYT), symbol(_symbol), weight(0),
    repr(boost::dynamic_bitset<>(_repr_size)) {}

void Node::_info(uint64_t *id, uint32_t *symbol, uint32_t *weight, Node **parent, bool *NYT)
{
    *id = this->id;
    *symbol = this->symbol;
    *weight = this->weight;
    *parent = this->parent;
    *NYT = this->NYT;
}

void Node::set(uint32_t _symbol)
{
    this->symbol = _symbol;
    this->NYT = false;
}

bool Node::external()
{
    return (this->left == nullptr && this->right == nullptr);
}

void Node::swap(Node *other)
{
    if (this == other) return;
    spdlog::debug("Swapping vals {},{} with {},{}",
            this->external() ? "e" : "i", this->id,
            other->external() ? "e" : "i", other->id);
    std::swap(this->id, other->id);
    std::swap(this->NYT, other->NYT);
    // swap the weight, even though the
    // weight should be the same
    std::swap(this->weight, other->weight);
    // std::swap(this->symbol, other->symbol);
    std::swap(this->repr, other->repr);
}
