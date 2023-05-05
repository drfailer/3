#ifndef __CONTEXT_MANAGER__
#define __CONTEXT_MANAGER__
#include <iostream>
#include <list>
#include "Symtable.hpp"

class ContextManager
{
  private:
    std::shared_ptr<Symtable> currentScope;

  public:
    void enterScope();
    void leaveScope();
    Symtable getScope() const; // NOTE: may be wrong
    void newSymbol(std::string name, std::list<Type> type, Kind kind);
    std::optional<Symbol> lookup(std::string name) const;
    ContextManager(): currentScope(std::make_shared<Symtable>()) {}
    ~ContextManager() = default;
};

#endif
