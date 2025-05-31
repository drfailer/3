#ifndef SYMTABLE_SYMBOL_TABLE_H
#define SYMTABLE_SYMBOL_TABLE_H
#include "symtable/symbol.hpp"
#include <map>
#include <string>

struct SymbolTable {
    SymbolTable *scope;
    std::map<std::string, Symbol> symbols;
    std::vector<SymbolTable*> childs_scopes;
};

Symbol *lookup(SymbolTable const *table, std::string const &id);

#endif
