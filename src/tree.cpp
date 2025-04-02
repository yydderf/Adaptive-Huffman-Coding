#include <spdlog/spdlog.h>
#include <cstdint>
#include <queue>

#include "node.h"
#include "tree.h"

Tree::Tree() :
    symbol_size(256),
    root(new Node(256 * 2 - 1)) {}

Tree::Tree(uint32_t _symbol_size) :
    symbol_size(_symbol_size),
    root(new Node(_symbol_size * 2 - 1)) {}

void Tree::update(char symbol)
{
}

void Tree::_info(uint32_t *symbol_size, Node **root)
{
    *symbol_size = this->symbol_size;
    *root = this->root;
}

std::queue<std::pair<Node*, int>> Tree::_get_queue()
{
    std::queue<std::pair<Node*, int>> ret_q;
    std::queue<std::pair<Node*, int>> q;
    std::pair<Node*, int> curr_pair;
    Node *left_node, *right_node;

    q.push(std::make_pair(this->root, 0));
    ret_q.push(std::make_pair(this->root, 0));

    while (!q.empty()) {
        curr_pair = q.front();
        if (curr_pair.first->left != nullptr) {
            q.push(std::make_pair(curr_pair.first->left, curr_pair.second));
            ret_q.push(std::make_pair(curr_pair.first->left, curr_pair.second));
        }
        if (curr_pair.first->right != nullptr) {
            q.push(std::make_pair(curr_pair.first->right, curr_pair.second));
            ret_q.push(std::make_pair(curr_pair.first->right, curr_pair.second));
        }
        q.pop();
    }
    return ret_q;
}

void Tree::display()
{
    std::queue<std::pair<Node*, int>> q = this->_get_queue();
    std::pair<Node*, int> curr_pair;
    uint64_t id;
    uint32_t symbol, weight;
    Node *parent;
    while (!q.empty()) {
        curr_pair = q.front();
        curr_pair.first->_info(&id, &symbol, &weight, &parent);
        spdlog::info("id: {} | symbol: {} | weight: {} | level: {}", id, symbol, weight, curr_pair.second);
        q.pop();
    }
}
