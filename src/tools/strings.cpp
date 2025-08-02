#include "strings.hpp"

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

