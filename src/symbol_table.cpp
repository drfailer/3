#include "symbol_table.hpp"
#include <iostream>

SymbolTable *symbol_table_create(SymbolTable *parent) {
    auto *table = new SymbolTable();
    table->parent = parent;
    if (parent != nullptr) {
        parent->childs_scopes.push_back(table);
    }
    return table;
}

void symbol_table_destroy(SymbolTable *table) {
    for (SymbolTable *child : table->childs_scopes) {
        symbol_table_destroy(child);
    }
    delete table;
}

void insert_symbol(SymbolTable *table, std::string const &id, Type *type,
                   SymbolTable *scope, Location const &location) {
    table->symbols.insert({id, Symbol{
                                   .id = id,
                                   .type = type,
                                   .parent_scope = table,
                                   .scope = scope,
                                   .location = location,
                               }});
}

Symbol *lookup_id(SymbolTable *table, std::string const &id) {
    auto maybe_symbol = table->symbols.find(id);
    if (maybe_symbol != table->symbols.end()) {
        return &maybe_symbol->second;
    } else if (table->parent != nullptr) {
        return lookup_id(table->parent, id);
    }
    return nullptr;
}

Type *lookup_ast_type(SymbolTable *table, Ast *ast) {
    auto maybe_type = table->ast_types.find(ast);
    if (maybe_type != table->ast_types.end()) {
        return maybe_type->second;
    } else if (table->parent != nullptr) {
        return lookup_ast_type(table->parent, ast);
    }
    return nullptr;
}
