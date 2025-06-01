#ifndef SYMTABLE_SYMBOL_TABLE_H
#define SYMTABLE_SYMBOL_TABLE_H
#include "symtable/symbol.hpp"
#include <map>
#include <string>
#include <vector>

struct SymbolTable {
    SymbolTable *parent;
    std::map<std::string, Symbol> symbols;
    std::vector<SymbolTable*> childs_scopes;
};

Symbol *lookup(SymbolTable *table, std::string const &id);

#endif
