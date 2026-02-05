#ifndef TOOLS_STRINGS
#define TOOLS_STRINGS
#include <string>
#include <cassert>

// TODO: rename this file to `string` and create a custom string type
// TODO: finish arena implementation
// TODO: add a custom array type for the ast
//
//
// BUG: we need a function to convert String std::string because the store string is not null terminated.
// NODE: at some point, std::string migh get completely replaced with String.

// a trivialy copyable/constructible type to use in the nodes
struct String {
    char *ptr;
    size_t len;
    // if we need to append to the string we need a cap
    // TODO: allocator

    char &operator[](size_t i) {
        assert(i < len && "error: index out of bound.");
        return ptr[i];
    }

    char const &operator[](size_t i) const {
        assert(i < len && "error: index out of bound.");
        return ptr[i];
    }
};

String string_create(std::string const &str);
String string_create(char const *cstr);
void   string_destroy(String *str);

bool starts_with(std::string const &str, std::string const &pattern);

#endif
