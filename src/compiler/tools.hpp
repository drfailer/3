#ifndef COMPILER_TOOLS
#define COMPILER_TOOLS
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <string>
#include <iostream>
#include <type_traits>
#include <vector>

namespace compiler {

size_t get_compiled_string_size(std::string const &str);

template <typename T> std::string ptr_to_string(T *ptr) {
    std::ostringstream oss;
    oss << ptr;
    return oss.str();
}

std::string base_name(std::string const &filename);
std::string asm_filename(std::string const &base_name);
std::string object_filename(std::string const &base_name);

template <typename T>
std::string format_cmd_arg(T const &arg) {
    std::ostringstream oss;

    if constexpr (std::is_same_v<std::vector<std::string>, T>) {
        for (auto arg_val : arg) {
            oss << " " << arg_val;
        }
    } else {
        oss << " " << arg;
    }
    return oss.str();
}

template <typename ...T>
int run_cmd(std::string const &exec, T ...args) {
    std::string cmd_str = (exec + ... + format_cmd_arg(args));
    std::cout << "running: " << cmd_str << std::endl;
    return system(cmd_str.c_str());
}

} // end namespace compiler

#endif
