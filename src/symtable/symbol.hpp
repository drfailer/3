#ifndef SYMBOL_H
#define SYMBOL_H
#include "type_system/types.hpp"

enum Kind { FUNCTION, FUN_PARAM, LOCAL_VAR, LOCAL_ARRAY };

class Symbol {
  public:
    Symbol(std::string const &name, type_system::type type, Kind kind)
        : name(name), type(type), kind(kind) {}
    Symbol() = default;

    std::string getName() const { return name; }
    type_system::type const getType() const { return type; }
    Kind getKind() const { return kind; }

  private:
    std::string name;
    type_system::type type;
    Kind kind;
};

#endif
