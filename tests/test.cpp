#include <execution>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <vector>
#include "./testlib/config.hpp"

#define PATH_TO_S3C "../build/s3c"

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

int main(int argc, char **argv) {
    // TODOs:
    // - parse command line options
    // * parse configs
    // - run the test
    //
    // Options:
    // - run all the tests
    // - run a specific test
    // - run all tests in a specific category
    // - run test for a particular platform (useless for now)
    // - list all test & their description (verbose output, selection, ...)
    // - update gold files for a specific test
    TestConfig config = parse_config_file("./config/cnd.conf");
    std::string base_name =
        config.files.src.substr(0, config.files.src.size() - 2);
    std::string out_dir  = "out/" + config.files.dir;
    std::string src_dir  = "src/" + config.files.dir;
    std::string exec  = out_dir + "/" + base_name;
    std::string out  = out_dir + "/" + base_name + ".out";
    std::string src  = src_dir + "/" + config.files.src;
    std::filesystem::create_directories(out_dir);
    run_cmd("../build/s3c", "-o", exec, src, config.compiler.flags, config.compiler.ldflags);
    run_cmd(exec, ">", out);
    return 0;
}
