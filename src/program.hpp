#ifndef PROGRAM
#define PROGRAM
#include "symbol_table.hpp"
#include "ast.hpp"
#include <vector>

struct Program {
    std::vector<Ast *> code;
    SymbolTable *scope;
};

#endif
