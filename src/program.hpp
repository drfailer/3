#ifndef PROGRAM
#define PROGRAM
#include "symbol_table.hpp"
#include "tree/node.hpp"
#include <vector>

struct Program {
    std::vector<node::Node *> code;
    SymbolTable *scope;
};

#endif
