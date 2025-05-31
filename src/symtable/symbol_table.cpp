#include "symbol_table.hpp"

Symbol *lookup(SymbolTable const *table, std::string const &id) {
    if (table->symbols.find(id) != table->symbols.end()) {
    } else if (table->scope != nullptr) {
        return lookup(table->scope, id);
    }
    return nullptr;
}
