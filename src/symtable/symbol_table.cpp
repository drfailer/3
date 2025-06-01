#include "symbol_table.hpp"

Symbol *lookup(SymbolTable *table, std::string const &id) {
    auto maybe_symbol = table->symbols.find(id);
    if (maybe_symbol != table->symbols.end()) {
        return &maybe_symbol->second;
    } else if (table->parent != nullptr) {
        return lookup(table->parent, id);
    }
    return nullptr;
}
