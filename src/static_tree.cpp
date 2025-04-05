#include "static_tree.h"
#include "node.h"
#include <queue>
#include <functional>
#include <spdlog/spdlog.h>

void StaticTree::construct(const std::vector<uint32_t> &frequencies) {
    // Priority queue for building the Huffman tree.
    std::priority_queue<Node*, std::vector<Node*>, StaticNodeDscComp> pq;

    // Clear any adaptive-specific state if needed.
    // For each symbol with nonzero frequency, create a leaf node.
    for (uint32_t sym = 0; sym < frequencies.size(); ++sym) {
        if (frequencies[sym] > 0) {
            // Create a leaf node.
            // We use the constructor: Node(uint64_t _id, uint32_t _symbol)
            // Here we let _id be 'sym' for uniqueness.
            Node* leaf = new Node(sym, sym);
            leaf->weight = frequencies[sym];
            pq.push(leaf);
        }
    }
    this->_construct(pq);
}

void StaticTree::construct(const std::unordered_map<uint32_t, size_t> &frequencies) {
    std::priority_queue<Node*, std::vector<Node*>, StaticNodeDscComp> pq;

    // Create a leaf for each unique symbol.
    for (const auto &entry : frequencies) {
        uint32_t symbol = entry.first;
        uint32_t freq = entry.second;
        if (freq > 0) {
            // Use the symbol value for the node id (or generate a unique id if needed).
            Node* leaf = new Node(symbol, symbol);
            leaf->weight = freq;
            pq.push(leaf);
        }
    }
    this->_construct(pq);
}

void StaticTree::_construct(std::priority_queue<Node*, std::vector<Node*>, StaticNodeDscComp> &pq)
{
    if (pq.empty()) {
        spdlog::warn("No symbols with nonzero frequency found.");
        this->root = nullptr;
        return;
    }

    // Edge case: if only one symbol is present, add a dummy node.
    if (pq.size() == 1) {
        Node* only = pq.top();
        pq.pop();
        // Create a dummy leaf with zero frequency.
        Node* dummy = new Node(only->symbol, only->symbol);
        dummy->weight = 0;
        // Create a parent node.
        Node* parent = new Node(0, 0, 0, nullptr, false);
        parent->left = only;
        parent->right = dummy;
        only->parent = parent;
        dummy->parent = parent;
        parent->weight = only->weight; // frequency of the single symbol.
        pq.push(parent);
    }

    // Build the tree: combine two smallest nodes repeatedly.
    while (pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();
        // Create an internal node.
        Node* parent = new Node(0, 0, 0, nullptr, false);
        parent->left = left;
        parent->right = right;
        left->parent = parent;
        right->parent = parent;
        parent->weight = left->weight + right->weight;
        pq.push(parent);
    }
    // The remaining node is the root of the static Huffman tree.
    this->root = pq.top();
    this->_get_queue(this->root, nullptr);
}
