#include "ast.hpp"
#include <cstring>
#include <iostream>

Location location_create(std::string const &filename, size_t row, size_t col) {
    return Location{
        .row = row,
        .col = col,
        .filename = filename,
    };
}

std::ostream &operator<<(std::ostream &os, Location const &location) {
    os << location.filename << '(' << location.row << ':' << location.col << ')';
    return os;
}
std::string operator_name(ArithmeticOperationKind kind) {
    switch (kind) {
    case ArithmeticOperationKind::Add: return "add"; break;
    case ArithmeticOperationKind::Sub: return "sub"; break;
    case ArithmeticOperationKind::Mul: return "mul"; break;
    case ArithmeticOperationKind::Div: return "div"; break;
    }
    return "unknown operator";
}

void print_ast(Ast *) {
    std::cerr << "error: print_ast not implemented" << std::endl;
}

#undef create_ast
