#include "context_manager.hpp"

void ContextManager::enterScope() {
    auto newScope = new SymbolTable();
    newScope->parent = currentScope;
    currentScope->childs_scopes.push_back(newScope);
    currentScope = newScope;
}

void ContextManager::leaveScope() { currentScope = currentScope->parent; }

void ContextManager::newSymbol(std::string const &id, type::Type *type) {
    currentScope->symbols.insert({id, Symbol{
                                          .id = id,
                                          .type = type,
                                          .scope = currentScope,
                                      }});
}

/**
 * trick for functions
 */
void ContextManager::newGlobalSymbol(std::string const &id, type::Type *type) {
    globalScope->symbols.insert({id, Symbol{
            .id = id,
            .type = type,
            .scope = globalScope,
            }});
}

Symbol *ContextManager::lookup(std::string const &id) const {
    return ::lookup(currentScope, id);
}
