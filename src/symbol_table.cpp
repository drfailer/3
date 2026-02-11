#include "symbol_table.hpp"

SymbolTable *symbol_table_create(SymbolTable *parent) {
    auto *table = new SymbolTable();
    table->parent = parent;
    return table;
}

void insert_symbol(SymbolTable *table, std::string const &id, type::Type *type,
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

type::Type *lookup_ast_type(SymbolTable *table, Ast *ast) {
    auto maybe_type = table->ast_types.find(ast);
    if (maybe_type != table->ast_types.end()) {
        return maybe_type->second;
    } else if (table->parent != nullptr) {
        return lookup_ast_type(table->parent, ast);
    }
    return nullptr;
}
