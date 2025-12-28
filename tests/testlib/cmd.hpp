#ifndef TESTLIB_CMD
#define TESTLIB_CMD
#include <execution>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

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

#endif
