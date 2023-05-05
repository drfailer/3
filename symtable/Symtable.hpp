#ifndef __SYMTABLE__
#define __SYMTABLE__
#include <iostream>
#include <memory>
#include <unordered_map>
#include <list>
#include <optional>
#include "Symbol.hpp"

class Symtable
{
  private:
    std::unordered_map<std::string, Symbol> table;
    std::list<std::shared_ptr<Symtable>> childScopes;
    std::shared_ptr<Symtable> father; // father node in the table

  public:
    std::list<std::shared_ptr<Symtable>> getChildScopes() const;
    std::shared_ptr<Symtable> getFather() const;
    std::optional<Symbol> lookup(std::string name);
    void addScope(std::shared_ptr<Symtable>);
    void add(std::string name, std::list<Type> type, Kind kind);
    Symtable(std::shared_ptr<Symtable>);
    Symtable() = default;
    ~Symtable() = default;
};

#endif
