#ifndef SYMBOL_H
#define SYMBOL_H
#include "typesystem/types.hpp"
#include <list>

enum Kind {
    FUNCTION,
    FUN_PARAM,
    LOCAL_VAR,
    LOCAL_ARRAY
};

class Symbol {
  public:
    Symbol(std::string name, std::list<Type> type, Kind kind);
    Symbol(std::string name, std::list<Type> type, unsigned int size,
           Kind kind);
    Symbol() = default;

    std::string getName() const;
    std::list<Type> getType() const;
    int getSize() const { return size; }
    Kind getKind() const;

  private:
    std::string name;
    std::list<Type> type;
    unsigned int size; // size for arrays
    Kind kind;
};

#endif
