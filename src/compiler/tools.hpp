#ifndef COMPILER_TOOLS
#define COMPILER_TOOLS
#include <cstddef>
#include <sstream>
#include <string>

namespace compiler {

size_t get_compiled_string_size(std::string const &str);

template <typename T> std::string ptr_to_string(T *ptr) {
    std::ostringstream oss;
    oss << ptr;
    return oss.str();
}

} // end namespace compiler

#endif
