#ifndef COMPILER_TOOLS
#define COMPILER_TOOLS
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <string>
#include <iostream>

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

template <typename ...T>
void run_cmd(std::string const &exec, T ...args) {
    std::string cmd_str = (exec + ... + (std::string(" ") + args));
    std::cout << "running: " << cmd_str << std::endl;
    system(cmd_str.c_str());
}

} // end namespace compiler

#endif
