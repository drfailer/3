#ifndef SYMTABLE_SYMBOL_TABLE_H
#define SYMTABLE_SYMBOL_TABLE_H
#include "ast.hpp"
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
    SymbolTable *parent_scope;
    SymbolTable *scope;
    Location location;
};

struct SymbolTable {
    SymbolTable *parent;
    std::map<std::string, Symbol> symbols;
    std::vector<SymbolTable *> childs_scopes;
    std::map<Ast*, SymbolTable*> block_scopes;
    std::map<Ast*, type::Type*> ast_types; // evaluated type of
                                                   // expression asts
};

SymbolTable *symbol_table_create(SymbolTable *parent);
void insert_symbol(SymbolTable *table, std::string const &id, type::Type *type,
                   SymbolTable *scope, Location const &location);
Symbol *lookup_id(SymbolTable *table, std::string const &id);
type::Type *lookup_ast_type(SymbolTable *table, Ast *ast);

#endif
