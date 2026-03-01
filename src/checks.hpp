#ifndef CHECKS
#define CHECKS
#include <iostream>
#include <vector>
#include "ast.hpp"
#include "scope.hpp"
#include "program.hpp"
#include "tools/mem.hpp"

/*
 * This module contains function that verify the types and the symbol
 * definitions.
 */

struct CheckState {
    Ast *curr_function;
    MemPool<Type> *type_pool;
    Allocator allocator;
    Scope *global_scope;
};

bool check(CheckState *state, Program const &program);

#endif
