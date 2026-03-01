#include "tools.hpp"
#include <algorithm>
#include <filesystem>

namespace compiler {

size_t get_compiled_string_size(std::string const &str) {
    size_t size = str.size() - std::count_if(str.begin(), str.end(),
                                             [](char c) { return c == '\\'; });
    return size - 2; // do not cout quotes
}

std::string base_name(std::string const &filename) {
    std::string base_name = std::filesystem::path(filename).filename();
    return base_name.substr(0, base_name.size() - 2); // remove .3
}

std::string asm_filename(std::string const &base_name) {
    return base_name + ".asm";
}

std::string object_filename(std::string const &base_name) {
    return base_name + ".o";
}

} // end namespace compiler
