#include "tools.hpp"
#include <algorithm>

namespace compiler {

size_t get_compiled_string_size(std::string const &str) {
    return str.size() - std::count_if(str.begin(), str.end(),
                                      [](char c) { return c == '\\'; });
}

} // end namespace compiler
