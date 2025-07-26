#include "tools.hpp"
#include <algorithm>

namespace compiler {

size_t get_compiled_string_size(std::string const &str) {
    size_t size = str.size() - std::count_if(str.begin(), str.end(),
                                             [](char c) { return c == '\\'; });
    return size - 2; // do not cout quotes
}

} // end namespace compiler
