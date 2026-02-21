#ifndef CHECKS
#define CHECKS
#include <iostream>
#include <vector>
#include "ast.hpp"
#include "symbol_table.hpp"
#include "tools/mem.hpp"

/*
 * This module contains function that verify the types and the symbol
 * definitions.
 */

struct CheckState {
    std::string current_function; // TODO: use the ast node here instead of a string
    MemPool<Type> *type_pool;
    Allocator allocator;
};

bool check(CheckState *state, std::vector<Ast *> const &program, SymbolTable *symtable);

#endif
