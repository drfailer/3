#ifndef __CONTEXT_MANAGER__
#define __CONTEXT_MANAGER__
#include "Symtable.hpp"
#include <iostream>
#include <list>

class ContextManager {
      public:
        void enterScope();
        void leaveScope();
        Symtable getScope() const; // NOTE: may be wrong
        void newSymbol(std::string name, std::list<Type> type, Kind kind);
        std::optional<Symbol> lookup(std::string name) const;
        ContextManager() : currentScope(std::make_shared<Symtable>()) {}
        ~ContextManager() = default;

      private:
        std::shared_ptr<Symtable> currentScope;
};

#endif
