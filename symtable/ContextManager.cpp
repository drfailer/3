#include "ContextManager.hpp"
#include <sstream>

Symtable ContextManager::getScope() const { return *currentScope; }

void ContextManager::enterScope() {
        std::shared_ptr<Symtable> newScope =
            std::make_shared<Symtable>(currentScope);
        currentScope->addScope(newScope);
        currentScope = newScope;
}

void ContextManager::leaveScope() { currentScope = currentScope->getFather(); }

void ContextManager::newSymbol(std::string name, std::list<Type> type,
                               Kind kind) {
        currentScope->add(name, type, kind);
}

void ContextManager::newSymbol(std::string name, std::list<Type> type, unsigned int size,
                               Kind kind) {
        currentScope->add(name, type, size, kind);
}

std::optional<Symbol> ContextManager::lookup(std::string name) const {
        return currentScope->lookup(name);
}
