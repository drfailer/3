#ifndef PROGRAM
#define PROGRAM
#include "symbol_table.hpp"
#include "tree/node.hpp"
#include <list>

struct Program {
    std::list<node::Node *> code;
    SymbolTable *scope;
};

#endif
