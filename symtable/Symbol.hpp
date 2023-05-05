#ifndef __SYMBOL__
#define __SYMBOL__
#include <iostream>
#include <list>
#include "../AST/Types.hpp"

enum Kind
{
  FUN_PARAM,
  LOCAL_VAR,
  LOCAL_ARRAY,
  FUNCTION
};

class Symbol
{
  private:
    std::string name;
    std::list<Type> type;
    Kind kind;

  public:
    Kind getKind() const;
    std::list<Type> getType() const;
    std::string getName() const;
    Symbol(std::string name, std::list<Type> type, Kind kind);
    Symbol() = default;
    ~Symbol() = default;
};

#endif
