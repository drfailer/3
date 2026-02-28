#include "scope.hpp"
#include <algorithm>

Scope *scope_create(Scope *parent) {
    auto scope = new Scope();
    scope->parent = parent;
    return scope;
}

// Since we use maps, we won't use an arena for the scope
void scope_destroy(Scope *scope) {
    for (auto &child : scope->child_scopes) {
        scope_destroy(child.second);
    }
    delete scope;
}

Symbol *scope_add_symbol(Scope *scope, std::string const &name, Type *type, Location const &location) {
    scope->symbol_table[name] = Symbol{
        .type = type,
        .scope = scope,
        .location = location,
    };
    return &scope->symbol_table[name];
}

TypeInfo *scope_add_type(Scope *scope, std::string const &name, Type *type, Location const &location) {
    scope->type_table[name] = TypeInfo{
        .type = type,
        .scope = scope,
        .size = size_of(type),
        .location = location,
    };
    return &scope->type_table[name];
}

Scope *scope_add_child(Scope *scope, Ast *ast) {
    auto child = scope_create(scope);
    scope->child_scopes[ast] = child;
    return child;
}

Symbol *scope_lookup_symbol(Scope *scope, std::string const &symbol_name) {
    if (scope == nullptr) {
        return nullptr;
    }
    if (map_contains(scope->symbol_table, symbol_name)) {
        return &scope->symbol_table[symbol_name];
    }
    return scope_lookup_symbol(scope->parent, symbol_name);
}

TypeInfo *scope_lookup_type(Scope *scope, std::string const &type_name) {
    if (scope == nullptr) {
        return nullptr;
    }
    if (map_contains(scope->type_table, type_name)) {
        return &scope->type_table[type_name];
    }
    return scope_lookup_type(scope->parent, type_name);
}

Type *scope_lookup_expr_type(Scope *scope, Ast *expr) {
    if (scope == nullptr) {
        return nullptr;
    }
    if (map_contains(scope->expr_types, expr)) {
        return scope->expr_types[expr];
    }
    return scope_lookup_expr_type(scope->parent, expr);
}
