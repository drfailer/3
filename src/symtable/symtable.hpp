#ifndef SYMTABLE_H
#define SYMTABLE_H
#include "symbol.hpp"
#include <memory>
#include <optional>
#include <unordered_map>

// note: the kind is embedded in the type but it should be accessed with the
// symbol

class Symtable {
  public:
    Symtable(std::shared_ptr<Symtable>);
    Symtable() = default;
    ~Symtable() = default;

    std::list<std::shared_ptr<Symtable>> getChildScopes() const;
    std::shared_ptr<Symtable> getFather() const;
    std::optional<Symbol> lookup(std::string name);
    void addScope(std::shared_ptr<Symtable>);
    void add(std::string const &name, type_system::type_t type, Kind kind);

  private:
    std::unordered_map<std::string, Symbol> table;
    std::list<std::shared_ptr<Symtable>> childScopes;
    std::shared_ptr<Symtable> father; // father node in the table
};

#endif
