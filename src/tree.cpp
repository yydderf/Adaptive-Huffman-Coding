#include <spdlog/spdlog.h>
#include <unordered_map>
#include <cstdint>
#include <queue>

#include <cstdio>

#include "node.h"
#include "tree.h"

Tree::Tree() :
    symbol_size(256),
    symbol_trans(0),
    NYT(boost::dynamic_bitset<>(0)),
    root(new Node(256 * 2 - 1))
{
    this->node_NYT = root;
}

Tree::Tree(uint32_t _symbol_size) :
    symbol_size(_symbol_size),
    symbol_trans(0),
    NYT(boost::dynamic_bitset<>(0)),
    root(new Node(_symbol_size * 2 - 1))
{
    this->node_NYT = root;
}

Tree::~Tree()
{
    // free all the nodes in the tree
    // including root and node_NYT
    std::queue<std::pair<Node*, int>> q = this->_get_queue(this->root);
    std::pair<Node*, int> curr_pair;
    while (!q.empty()) {
        curr_pair = q.front();
        q.pop();
        delete curr_pair.first;
    }
}

void Tree::update(uint32_t symbol)
{
    // check if the symbol had appeared before
    Node *curr_node = this->search(symbol);
    // - NYT gives birth to new NYT and external node
    // - increment weight of the external node and old NYT
    // - to old NYT
    if (curr_node == nullptr) {
        Node *new_symbol = new Node(symbol, this->node_NYT, NODE_RIGHT);
        Node *new_NYT = new Node(symbol, this->node_NYT, NODE_LEFT);

        // update weight
        this->node_NYT->weight += 1;
        new_symbol->weight += 1;

        // update table
        symbol_map[symbol] = new_symbol;

        // set new NYT as node NYT
        curr_node = this->node_NYT;
        this->node_NYT = new_NYT;

        this->symbol_trans += 1;
    } else {
        // node number max in block
        this->_switch(curr_node);
        curr_node->weight += 1;
    }

    while (curr_node != this->root) {
        // * check if the current node is root
        // * go to parent node
        curr_node = curr_node->parent;
        this->_switch(curr_node);
        curr_node->weight += 1;
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
    auto it = this->block_map.find(node->weight);
    // 1. no nodes with the weight have registered
    if (it != this->block_map.end()) {
        this->block_map[node->weight] = std::vector<Node*>();
        this->block_map[node->weight].push_back(node);
    } else {
        // get max_id, switch node with max_id node
        // remove node from old_weight container
        // push node to new_weight container
        // std::vector fetch in O(N), remove in O(N)
        // std::priority_queue fetch in O(log(N)), remove in O(?)
        // 2. target weight has been registered, node != max_id
        // 3. target weight has been registered, node == max_id
        // Node *max_node = node;
        // for (int i = 0; i < it->second.size(); ++i) {
        //     if (it->second[i]->id > max_node->id && it->second[i] == node->parent) {
        //         max_node = it->second[i];
        //     }
        // }
        // if (max_node != node) {
        //     auto tmp_node = max_node;
        //     auto tmp_parent = max_node->parent;
        //     auto tmp_id = max_node->id;
        //     auto tmp_NYT = max_node->NYT;
        //     auto tmp_repr = max_node->repr;

        //     max_node = node;
        //     max_node->id = tmp_id;
        //     max_node->parent = tmp_parent;
        //     max_node->NYT = tmp_NYT;
        //     max_node->repr = tmp_repr;

        //     node = tmp_node;
        // }
    }
    
    // if (it != this->block_map.end() && it->second != node) {
    // }
    // this->block_map[node->weight] = node;
}

void Tree::_info(uint32_t *symbol_size, Node **root)
{
    *symbol_size = this->symbol_size;
    *root = this->root;
}

std::queue<std::pair<Node*, int>> Tree::_get_queue(Node *node)
{
    std::queue<std::pair<Node*, int>> ret_q;
    std::queue<std::pair<Node*, int>> q;
    std::pair<Node*, int> curr_pair;
    Node *left_node, *right_node;

    q.push(std::make_pair(node, 0));
    ret_q.push(std::make_pair(node, 0));

    while (!q.empty()) {
        curr_pair = q.front();
        if (curr_pair.first->left != nullptr) {
            q.push(std::make_pair(curr_pair.first->left, curr_pair.second + 1));
            ret_q.push(std::make_pair(curr_pair.first->left, curr_pair.second + 1));
        }
        if (curr_pair.first->right != nullptr) {
            q.push(std::make_pair(curr_pair.first->right, curr_pair.second + 1));
            ret_q.push(std::make_pair(curr_pair.first->right, curr_pair.second + 1));
        }
        q.pop();
    }
    return ret_q;
}

void Tree::display()
{
    this->display(this->root);
}

void Tree::display(Node *node)
{
    std::queue<std::pair<Node*, int>> q = this->_get_queue(node);
    std::pair<Node*, int> curr_pair;
    uint64_t id;
    uint32_t symbol, weight;
    bool NYT;
    Node *parent;
    while (!q.empty()) {
        curr_pair = q.front();
        curr_pair.first->_info(&id, &symbol, &weight, &parent, &NYT);
        if (curr_pair.first->external() && !curr_pair.first->NYT) {
            spdlog::info("id: {} | type: e | weight: {} | level: {} | symbol: {}", id, weight, curr_pair.second, symbol);
        } else {
            spdlog::info("id: {} | type: i | weight: {} | level: {}", id, weight, curr_pair.second);
        }
        q.pop();
    }
}

Node *Tree::search(uint32_t symbol)
{
    // use unordered map to get search for symbol in O(1)
    // otherwise each update is at least O(N)
    auto it = this->symbol_map.find(symbol);
    return (it != this->symbol_map.end()) ? it->second : nullptr;
}
