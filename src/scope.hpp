#ifndef SCOPE
#define SCOPE
#include "ast.hpp"
#include "type.hpp"
#include "tools/mem.hpp"
#include <map>
#include <string>

struct Scope;

struct Symbol {
    Type *type;
    Scope *scope;
    Location location;
};

struct TypeInfo {
    Type *type;
    Scope *scope;
    size_t size;
    Location location; // empty for primitive types
};

struct Scope {
    Scope *parent;
    std::map<Ast*, Scope*> child_scopes;        // childs scopes indexed by ast nodes (blocks and functions nodes)
    std::map<std::string, Symbol> symbol_table; // table of symbls
    std::map<std::string, TypeInfo> type_table; // table of types (obj, enm, uni)
    std::map<Ast*, Type*> expr_types;           // evaluated types of expressions (filled during type checking)
};

// TODO: we should init the first scope in s3c.cpp, adding primitive types infos

Scope *scope_create(Scope *parent = nullptr);
void scope_destroy(Scope *scope);

Symbol   *scope_add_symbol(Scope *scope, std::string const &name, Type *type, Location const &location);
TypeInfo *scope_add_type(Scope *scope, std::string const &name, Type *type, Location const &location = {});

Scope  *scope_add_child(Scope *scope, Ast *ast);

Symbol   *scope_lookup_symbol(Scope *scope, std::string const &symbol_name);
TypeInfo *scope_lookup_type(Scope *scope, std::string const &type_name);
Type     *scope_lookup_expr_type(Scope *scope, Ast *expr);

// contains helper function for maps (the contains method was added in C++20,
// but the project uses C++17)
template <typename K, typename V>
bool map_contains(std::map<K, V> const &m, K const &key) {
    return m.find(key) != m.end();
}

#endif
