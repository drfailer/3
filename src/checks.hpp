#ifndef CHECKS
#define CHECKS
#include <iostream>
#include <vector>
#include "ast.hpp"
#include "symbol_table.hpp"

/*
 * This module contains function that verify the types and the symbol
 * definitions.
 */

struct CheckState {
    std::string current_file;
    std::string current_function;
};

bool check(std::vector<Ast *> program, SymbolTable *symtable);

#endif
