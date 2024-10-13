#ifndef SYMTABLE_H
#define SYMTABLE_H
#include "symbol.hpp"
#include <list>
#include <memory>
#include <optional>
#include <unordered_map>

class Symtable {
  public:
    Symtable(std::shared_ptr<Symtable>);
    Symtable() = default;
    ~Symtable() = default;

    std::list<std::shared_ptr<Symtable>> getChildScopes() const;
    std::shared_ptr<Symtable> getFather() const;
    std::optional<Symbol> lookup(std::string name);
    void addScope(std::shared_ptr<Symtable>);
    void add(std::string name, std::list<PrimitiveType> type, Kind kind);
    void add(std::string name, std::list<PrimitiveType> type, unsigned int size,
             Kind kind);

  private:
    std::unordered_map<std::string, Symbol> table;
    std::list<std::shared_ptr<Symtable>> childScopes;
    std::shared_ptr<Symtable> father; // father node in the table
};

#endif
