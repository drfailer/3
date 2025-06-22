#ifndef SYMTABLE_SYMBOL_TABLE_H
#define SYMTABLE_SYMBOL_TABLE_H
#include "tree/location.hpp"
#include "type/type.hpp"
#include <map>
#include <string>
#include <vector>

struct SymbolTable;
struct Symbol {
    std::string id; // TODO: create an identifier pool and store the id in a
                    // char const*
    type::Type *type; // TODO: a symbol can be a custom type -> we should have a
                      // kind here
    SymbolTable *scope;
    Location location;
};

struct SymbolTable {
    SymbolTable *parent;
    std::map<std::string, Symbol> symbols;
    std::vector<SymbolTable *> childs_scopes;
};

SymbolTable *symbol_table_create(SymbolTable *parent);
void insert_symbol(SymbolTable *table, std::string const &id, type::Type *type,
                   Location const &location);
Symbol *lookup(SymbolTable *table, std::string const &id);

#endif
