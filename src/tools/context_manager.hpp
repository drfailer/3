#ifndef CONTEXT_MANAGER_H
#define CONTEXT_MANAGER_H
#include "symtable/symbol_table.hpp"

namespace type {
struct Type;
}

struct ContextManager {
    SymbolTable *currentScope = nullptr;
    SymbolTable *globalScope = nullptr;

    ContextManager()
        : currentScope(new SymbolTable()), globalScope(currentScope) {}

    void enterScope();
    void leaveScope();
    void newSymbol(std::string const &id, type::Type *type);
    void newGlobalSymbol(std::string const &id, type::Type *type);
    Symbol *lookup(std::string const &id) const;
};

#endif
