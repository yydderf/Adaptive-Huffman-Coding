#include <boost/dynamic_bitset.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <sstream>
#include <cstdint>
#include <queue>
#include <set>

#include <cstdio>

#include "node.h"
#include "tree.h"

Tree::Tree() :
    root(new Node(256 * 2 - 1)),
    symbol_size(256),
    symbol_trans(0)
{
    this->node_NYT = root;
}

Tree::Tree(uint32_t _symbol_size) :
    root(new Node(_symbol_size * 2 - 1)),
    symbol_size(_symbol_size),
    symbol_trans(0)
{
    this->node_NYT = root;
}

Tree::~Tree()
{
    // free all the nodes in the tree
    // including root and node_NYT
    std::queue<std::pair<Node*, int>> q;
    this->_get_queue(this->root, &q);
    std::pair<Node*, int> curr_pair;
    while (!q.empty()) {
        curr_pair = q.front();
        q.pop();
        delete curr_pair.first;
    }
}

void Tree::update(Node *curr_node, uint32_t symbol)
{
    // check if the symbol had appeared before
    // - NYT gives birth to new NYT and external node
    // - increment weight of the external node and old NYT
    // - to old NYT
    spdlog::debug("tree update");
    if (curr_node == nullptr) {
        Node *new_symbol = new Node(symbol, this->node_NYT, NODE_RIGHT);
        Node *new_NYT = new Node(symbol, this->node_NYT, NODE_LEFT);

        // update weight
        this->node_NYT->weight += 1;
        new_symbol->weight += 1;

        // update table
        this->symbol_map[symbol] = new_symbol;

        this->block_map[this->node_NYT->weight].insert(this->node_NYT);
        this->block_map[new_symbol->weight].insert(new_symbol);

        // set new NYT as node NYT
        curr_node = this->node_NYT;
        this->node_NYT = new_NYT;

        this->symbol_trans += 1;
    } else {
        // node number max in block
        this->_switch(curr_node);
        this->_remove_from_set(curr_node);
        curr_node->weight += 1;
        this->block_map[curr_node->weight].insert(curr_node);
    }

    while (curr_node != this->root) {
        // * check if the current node is root
        // * go to parent node
        spdlog::debug("from {} to its parent {}", curr_node->id, curr_node->parent->id);
        curr_node = curr_node->parent;
        this->_switch(curr_node);
        this->_remove_from_set(curr_node);
        curr_node->weight += 1;
        this->block_map[curr_node->weight].insert(curr_node);
    }
    this->_get_queue(this->root, nullptr);
}

void Tree::_remove_from_set(Node *target_node)
{
    auto nit = std::find_if(
            this->block_map[target_node->weight].begin(),
            this->block_map[target_node->weight].end(),
            [target_id = target_node->id](const Node * n) {
                return n->id == target_id;
            });
    if (nit != this->block_map[target_node->weight].end()) {
        this->block_map[target_node->weight].erase(nit);
    }
}

void Tree::_switch(Node *node)
{
    // 1. get the max_id node from container
    // 2. switch node with max_id node
    // 2.1 - swap node
    // 2.2 - update parent, id, NYT?, repr
    // 2.3 - reconstruction might not be required
    //       the representation of each node does
    //       tie with the node number (id)
    spdlog::debug("_switch - id: {}", node->id);
    spdlog::debug("finding the largest node number of weight: {}", node->weight);
    auto it = this->block_map.find(node->weight);
    // 1. no nodes with the weight have registered
    if (it == this->block_map.end()) {
        this->block_map[node->weight] = std::set<Node*, NodeAscComp>();
        this->block_map[node->weight].insert(node);
        return;
    }
    // get max_id, switch node with max_id node
    // remove node from old_weight container
    // push node to new_weight container
    // std::vector fetch in O(N), remove in O(N)
    // std::priority_queue fetch in O(log(N)), remove in O(?)
    // 2. target weight has been registered, node != max_id
    // 3. target weight has been registered, node == max_id
    auto rit = it->second.rbegin();
    if (*rit == node->parent) {
        auto next_rit = std::next(rit);
        if (next_rit == it->second.rend()) {
            spdlog::debug("no candidate found for swapping");
            return;
        }
        rit = next_rit;
    }

    Node *max_node = *rit;
    spdlog::debug("max node id: {}", max_node->id);
    this->swap(node, max_node);
}

void Tree::_info(uint32_t *symbol_size, Node **root)
{
    *symbol_size = this->symbol_size;
    *root = this->root;
}

void Tree::_get_queue(Node *node, std::queue<std::pair<Node *, int>> *rq)
{
    // _get_queue will also reconstruct repr
    std::queue<std::pair<Node*, int>> q;
    std::pair<Node*, int> curr_pair;

    q.push(std::make_pair(node, 0));
    if (rq != nullptr) rq->push(std::make_pair(node, 0));

    while (!q.empty()) {
        curr_pair = q.front();
        auto repr_size = curr_pair.first->repr.size();
        if (curr_pair.first->left != nullptr) {
            q.push(std::make_pair(curr_pair.first->left, curr_pair.second + 1));
            if (rq != nullptr) rq->push(std::make_pair(curr_pair.first->left, curr_pair.second + 1));
            curr_pair.first->left->repr = curr_pair.first->repr;
            curr_pair.first->left->repr.resize(repr_size + 1);
            curr_pair.first->left->repr[repr_size] = 1;
        }
        if (curr_pair.first->right != nullptr) {
            q.push(std::make_pair(curr_pair.first->right, curr_pair.second + 1));
            if (rq != nullptr) rq->push(std::make_pair(curr_pair.first->right, curr_pair.second + 1));
            curr_pair.first->right->repr = curr_pair.first->repr;
            curr_pair.first->right->repr.resize(repr_size + 1);
            curr_pair.first->right->repr[repr_size] = 0;
        }
        q.pop();
    }
}

void Tree::display()
{
    this->display(this->root);
}

void Tree::display(Node *node)
{
    std::queue<std::pair<Node*, int>> q;
    this->_get_queue(node, &q);
    std::pair<Node*, int> curr_pair;
    uint64_t id;
    uint32_t symbol, weight;
    bool NYT;
    Node *parent;
    while (!q.empty()) {
        curr_pair = q.front();
        curr_pair.first->_info(&id, &symbol, &weight, &parent, &NYT);
        if (curr_pair.first->NYT) {
            spdlog::info("id: {} | type: e | weight: {} | level: {} | symbol: NYT",
                    id, weight, curr_pair.second);
        } else if (curr_pair.first->external()) {
            spdlog::info("id: {} | type: e | weight: {} | level: {} | symbol: {}",
                    id, weight, curr_pair.second, symbol);
        } else {
            spdlog::info("id: {} | type: i | weight: {} | level: {} | left_id: {} | right_id: {}",
                    id, weight, curr_pair.second,
                    curr_pair.first->left->id,
                    curr_pair.first->right->id);
        }
        std::string buf;
        std::stringstream ss;
        boost::to_string(curr_pair.first->repr, buf);
        ss << buf;
        if (curr_pair.first->parent != nullptr) {
            ss << " | parent: " << curr_pair.first->parent->id;
        }
        spdlog::info("\trepr: {}", ss.str());
        q.pop();
    }
}

Node *Tree::search(uint32_t symbol)
{
    // use unordered map to get search for symbol in O(1)
    // otherwise each update is at least O(N)
    spdlog::debug("symbol: {}", symbol);
    auto it = this->symbol_map.find(symbol);
    return (it != this->symbol_map.end()) ? it->second : nullptr;
}

Node *Tree::get_NYT()
{
    return this->node_NYT;
}

Node *Tree::get_root()
{
    return this->root;
}

void Tree::swap(Node *node_a, Node *node_b)
{
    // id - odd == left | even == right
    // val - a | b
    // 0 - right | right
    // 1 - left | right
    // 2 - right | left
    // 3 - left | left
    if (node_a == node_b) return;
    spdlog::debug("Swapping nodes {},{} with {},{}",
            node_a->external() ? "e" : "i", node_a->id,
            node_b->external() ? "e" : "i", node_b->id);
    auto val = ((node_a->id & 1) | ((node_b->id & 1) << 1));
    switch (val) {
    case 0: std::swap(node_a->parent->right, node_b->parent->right); break;
    case 1: std::swap(node_a->parent->left, node_b->parent->right); break;
    case 2: std::swap(node_a->parent->right, node_b->parent->left); break;
    case 3: std::swap(node_a->parent->left, node_b->parent->left); break;
    }
    std::swap(node_a->parent, node_b->parent);
    node_a->swap(node_b);
}
