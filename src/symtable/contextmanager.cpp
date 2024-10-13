#include "contextmanager.hpp"

std::shared_ptr<Symtable> const ContextManager::getScope() const {
        return currentScope;
}

void ContextManager::enterScope() {
        auto newScope = std::make_shared<Symtable>(currentScope);
        currentScope->addScope(newScope);
        currentScope = newScope;
}

void ContextManager::leaveScope() { currentScope = currentScope->getFather(); }

void ContextManager::newSymbol(std::string name, std::list<PrimitiveType> type,
                               Kind kind) {
        currentScope->add(name, type, kind);
}

void ContextManager::newSymbol(std::string name, std::list<PrimitiveType> type,
                               unsigned int size, Kind kind) {
        currentScope->add(name, type, size, kind);
}

/**
 * trick for functions
 */
void ContextManager::newGlobalSymbol(std::string name, std::list<PrimitiveType> type,
                                     Kind kind) {
        globalScope->add(name, type, kind);
}

std::optional<Symbol> ContextManager::lookup(std::string name) const {
        return currentScope->lookup(name);
}
