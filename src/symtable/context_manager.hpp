#ifndef CONTEXT_MANAGER_H
#define CONTEXT_MANAGER_H
#include "symtable/symtable.hpp"

class ContextManager {
  public:
    ContextManager()
        : currentScope(std::make_shared<Symtable>()),
          globalScope(currentScope) {}

    void enterScope();
    void leaveScope();
    std::shared_ptr<Symtable> const getScope() const; // NOTE: may be wrong
    void newSymbol(std::string const &name, type_system::type type, Kind kind);
    void newSymbol(std::string const &name, type_system::type type,
                   unsigned int size, Kind kind);
    void newGlobalSymbol(std::string name, type_system::type type,
                         Kind kind);
    std::optional<Symbol> lookup(std::string name) const;

  private:
    std::shared_ptr<Symtable> currentScope = nullptr;
    std::shared_ptr<Symtable> globalScope = nullptr;
};

#endif
