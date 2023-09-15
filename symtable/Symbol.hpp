#ifndef __SYMBOL__
#define __SYMBOL__
#include "../AST/Types.hpp"
#include <iostream>
#include <list>

enum Kind { FUN_PARAM, LOCAL_VAR, LOCAL_ARRAY, FUNCTION };

class Symbol {
      public:
        std::string getName() const;
        std::list<Type> getType() const;
        int getSize() const { return size; }
        Kind getKind() const;
        Symbol(std::string name, std::list<Type> type, Kind kind);
        Symbol(std::string name, std::list<Type> type, unsigned int size,
               Kind kind);
        Symbol() = default;
        ~Symbol() = default;

      private:
        std::string name;
        std::list<Type> type;
        unsigned int size; // size for arrays
        Kind kind;
};

#endif
