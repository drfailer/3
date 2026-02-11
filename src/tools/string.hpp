#ifndef TOOLS_STRINGS
#define TOOLS_STRINGS
#include <string>
#include <cassert>
#include "mem.hpp"

// TODO: add support for arena alocator so that the memory get's released
// TODO?: replace std::string with this struct

// a trivialy copyable/constructible type to use in the asts
struct String {
    char *ptr;
    size_t len;
    // TODO: if we need to append to the string we need a cap
    Allocator allocator;

    char &operator[](size_t i) {
        assert(i < len && "error: index out of bound.");
        return ptr[i];
    }

    char const &operator[](size_t i) const {
        assert(i < len && "error: index out of bound.");
        return ptr[i];
    }
};

String string_create(std::string const &str, Allocator allocator = DEFAULT_ALLOCATOR);
String string_create(char const *cstr);
void   string_destroy(String *str);

bool starts_with(std::string const &str, std::string const &pattern);

#endif
