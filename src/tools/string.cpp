#include "string.hpp"
#include <cstring>

String string_create(std::string const &std_str, Allocator allocator) {
    size_t len = std_str.size();
    String str = {
        .ptr = alloc<char>(allocator, len + 1),
        .len = len,
        .allocator = allocator,
    };
    assert(str.ptr != nullptr && "error: failed to allocated string buffer.");
    memcpy(str.ptr, std_str.data(), len);
    str.ptr[str.len] = 0;
    return str;
}

void string_destroy(String *str) {
    free(str->allocator, str->ptr);
}

bool starts_with(std::string const &str, std::string const &pattern) {
    if (str.size() < pattern.size()) {
        return false;
    }
    for (size_t i = 0; i < pattern.size(); ++i) {
        if (str[i] != pattern[i]) {
            return false;
        }
    }
    return true;
}

