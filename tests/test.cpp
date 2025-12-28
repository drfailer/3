#include <cstring>
#include "./testlib/test.hpp"
#include <iostream>

void print_help() {
    std::cout << "Usage: test <opt> [<add>]" << std::endl;
    std::cout << std::endl;
    std::cout << "run [<pattern>]: run tests that match the given pattern (all if unspecified)" << std::endl;
    std::cout << "list [<pattern>]: list tests that match the given pattern (all if unspecified)" << std::endl;
    std::cout << "update [<pattern>]: update gold files for tests that match the given pattern (all if unspecified)" << std::endl;
}

int main(int argc, char **argv) {
    int arg_idx = 1;

    while (arg_idx < argc) {
        std::string arg(argv[arg_idx++]);
        std::string pattern;

        if (arg == "run") {
            if (arg_idx != argc) {
                pattern = std::string(argv[arg_idx++]);
            }
            run_tests(pattern);
        } else if (arg == "list") {
            if (arg_idx != argc) {
                pattern = std::string(argv[arg_idx++]);
            }
            list_tests(pattern);
        } else if (arg == "update") {
            if (arg_idx != argc) {
                pattern = std::string(argv[arg_idx++]);
            }
            update_tests_gold_files(pattern);
        } else {
            print_help();
        }
    }
    return 0;
}
