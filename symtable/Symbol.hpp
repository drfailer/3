#ifndef __SYMBOL__
#define __SYMBOL__
#include "../AST/Types.hpp"
#include <iostream>
#include <list>

enum Kind { FUN_PARAM, LOCAL_VAR, LOCAL_ARRAY, FUNCTION };

class Symbol {
      public:
        Kind getKind() const;
        std::list<Type> getType() const;
        std::string getName() const;
        Symbol(std::string name, std::list<Type> type, Kind kind);
        Symbol() = default;
        ~Symbol() = default;

      private:
        std::string name;
        std::list<Type> type;
        Kind kind;
};

#endif
