#ifndef TESTLIB_CONFIG
#define TESTLIB_CONFIG
#include <string>
#include <vector>
#include <filesystem>
#define CONFIG_EXTENSION ".conf"

struct TestConfig {
    std::string title;
    std::string category;
    std::string description;
    struct {
        std::string dir;
        std::string src;
    } files;
    struct {
        int exit_code;
        bool should_compile;
        bool should_run;
    } results;
    struct {
        std::vector<std::string> flags;
        std::vector<std::string> ldflags;
        std::vector<std::string> platforms;
    } compiler;
};

TestConfig parse_config_file(std::string const &filename);

std::vector<std::filesystem::path> get_config_files(std::string const &config_dir);

#endif
