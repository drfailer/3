#ifndef PROGRAM
#define PROGRAM
#include "scope.hpp"
#include "ast.hpp"
#include <vector>

struct Program {
    std::vector<Ast *> code;
    Scope *scope;
};

#endif
