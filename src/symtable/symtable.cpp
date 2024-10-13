#include "symtable.hpp"
#include <memory>

Symtable::Symtable(std::shared_ptr<Symtable> father) : father(father) {}

bool contains(std::unordered_map<std::string, Symbol> table, std::string name) {
    for (const auto &[key, value] : table) {
        if (key == name) {
            return true;
        }
    }
    return false;
}

/**
 * @brief  Lookup for a symbol in the symtable.
 *
 * @param  name  Name of the symbol to look up of for.
 * @param  scope  Current scope where the methode is called.
 *
 * NOTE: may be better if the symtable can interact with the contextManager
 *       directly
 *
 * NOTE: the scopes are added to the front so the variables declared later will
 *       be checked first. This improves performances and allow to find the
 *       right Symbol, in case where a name is used to define a function
 *       parameter and a local variable declared at the top of the function body
 *       (in that case, only the `kind` changes).
 *
 * @return  Optional symbol which is the symbol corresponding to `name` in the
 *          current scope.
 */
std::optional<Symbol> Symtable::lookup(std::string name) {
    if (contains(table, name)) {
        return table[name];
    } else {
        if (father != nullptr) // recherche dans la table supérieure
            return father->lookup(name);
    }
    return {};
}

/**
 * @brief  Add a new Symbol to the symtable.
 *
 * @param  name   Name of the new symbol.
 * @param  scope  Scope where the symbol has been declared.
 * @param  type   Type of the new symbol.
 * @param  kind   Kind of the new symbol (param, local variable, ...)
 */
void Symtable::add(std::string name, std::list<Type> type, Kind kind) {
    table[name] = Symbol(name, type, kind);
}

/**
 * @brief  Add a new Symbol to the symtable.
 *
 * @param  name   Name of the new symbol.
 * @param  scope  Scope where the symbol has been declared.
 * @param  type   Type of the new symbol.
 * @param  size   Size of the data (for arrays).
 * @param  kind   Kind of the new symbol (param, local variable, ...)
 */
void Symtable::add(std::string name, std::list<Type> type, unsigned int size,
                   Kind kind) {
    table[name] = Symbol(name, type, size, kind);
}

std::shared_ptr<Symtable> Symtable::getFather() const { return father; }

std::list<std::shared_ptr<Symtable>> Symtable::getChildScopes() const {
    return childScopes;
}

void Symtable::addScope(std::shared_ptr<Symtable> newScope) {
    childScopes.push_back(newScope);
}
