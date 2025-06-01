#ifndef SYMTABLE_SYMBOL_H
#define SYMTABLE_SYMBOL_H
#include <string>
#include "type.hpp"

struct SymbolTable;
struct Symbol {
    std::string id; // TODO: create an identifier pool and store the id in a
                    // char const*
    type::Type *type;
    SymbolTable *scope;
    // TODO: add location
};

#endif
