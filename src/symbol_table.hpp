#ifndef SYMTABLE_SYMBOL_TABLE_H
#define SYMTABLE_SYMBOL_TABLE_H
#include <map>
#include <string>
#include <vector>
#include "type/type.hpp"

struct SymbolTable;
struct Symbol {
    std::string id; // TODO: create an identifier pool and store the id in a
                    // char const*
    type::Type *type;
    SymbolTable *scope;
    // TODO: add location
};

struct SymbolTable {
    SymbolTable *parent;
    std::map<std::string, Symbol> symbols;
    std::vector<SymbolTable *> childs_scopes;
};

void insert_symbol(SymbolTable *table, std::string const &id, type::Type *type);
Symbol *lookup(SymbolTable *table, std::string const &id);

#endif
