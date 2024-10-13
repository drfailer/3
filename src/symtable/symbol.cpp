#include "symtable/symbol.hpp"

Symbol::Symbol(std::string name, std::list<Type> type, unsigned int size, Kind kind)
    : name(name), type(type), size(size), kind(kind) {}

Symbol::Symbol(std::string name, std::list<Type> type, Kind kind)
    : Symbol(name, type, 1, kind) {}

std::string Symbol::getName() const { return name; }

std::list<Type> Symbol::getType() const { return type; }

Kind Symbol::getKind() const { return kind; }
