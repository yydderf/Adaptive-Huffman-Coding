#include "tree.h"

Node::Node() :
    left(nullptr), right(nullptr), parent(nullptr), weight(0) {};

Tree::Tree() :
    root(new Node()) {};
