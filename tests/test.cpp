#include <execution>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <regex>
#include <algorithm>
#include <fstream>
#include <cassert>
#include "./testlib/config.hpp"

#define PATH_TO_S3C "../build/s3c"
#define CONFIG_DIR "config/"
#define OUT_DIR "out/"
#define SRC_DIR "src/"
#define GOLD_DIR "gold/"

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

void list_tests(std::string const &pattern = "") {
    auto paths = get_config_files(CONFIG_DIR);

    if (pattern.empty()) {
        for (auto path : paths) {
            std::cout << path << std::endl;
        }
    } else {
        std::regex re(pattern);

        for (auto path : paths) {
            if (std::regex_search(std::string(path), re)) {
                std::cout << path << std::endl;
            }
        }
    }
}

template <typename ...Ts>
void error(Ts ...args) {
    std::cout << "error: ";
    (std::cout << ... << args) << std::endl;
}

bool files_equal(std::string const& gold_path, std::string const& out_path) {
  std::ifstream fgold(gold_path);
  std::ifstream fout(out_path);
  assert(fgold.good());
  assert(fout.good());
  return std::equal(
          std::istreambuf_iterator<char>(fgold.rdbuf()),
          std::istreambuf_iterator<char>(),
          std::istreambuf_iterator<char>(fout.rdbuf()));
}

bool run_test(std::string const &path_to_config) {
    TestConfig config = parse_config_file(path_to_config);
    std::string base_name =
        config.files.src.substr(0, config.files.src.size() - 2);
    std::string out_dir  = OUT_DIR + config.files.dir;
    std::string src_dir  = SRC_DIR + config.files.dir;
    std::string gold_dir  = GOLD_DIR + config.files.dir;
    std::string exec  = out_dir + "/" + base_name;
    std::string out  = out_dir + "/" + base_name + ".out";
    std::string src  = src_dir + "/" + config.files.src;
    std::string gold  = gold_dir + "/" + base_name + ".out";

    std::filesystem::create_directories(out_dir);
    int build_res = run_cmd("../build/s3c", "-o", exec, src, config.compiler.flags, config.compiler.ldflags);
    if (build_res != 0 && config.results.should_compile) {
        error("failed to compile test '", config.title, "'.");
        return false;
    }
    if (!config.results.should_run) {
        return true;
    }

    int run_res = run_cmd(exec, ">", out);
    if (run_res != config.results.exit_code) {
        error("wrong exit code for program `", config.title, "', expected ",
              config.results.exit_code, " but found ", run_res, ".");
        return false;
    }

    if (!std::filesystem::exists(gold)) {
        error("no gold files found for test `", config.title, "'");
        return false;
    }
    if (!files_equal(gold, out)) {
        error("out file does not correspond to gold file for test `", config.title, "'.");
        return false;
    }
    return true;
}

void run_all_tests() {
    // TODO
}

void update_gold_files(std::string const &path_to_config) {
    TestConfig config = parse_config_file(path_to_config);
    std::string base_name =
        config.files.src.substr(0, config.files.src.size() - 2);
    std::string src_dir  = SRC_DIR + config.files.dir;
    std::string out_dir  = OUT_DIR + config.files.dir;
    std::string gold_dir  = GOLD_DIR + config.files.dir;
    std::string exec  = out_dir + "/" + base_name;
    std::string src  = src_dir + "/" + config.files.src;
    std::string gold  = gold_dir + "/" + base_name + ".out";

    std::cout << "update gold file for test '" << config.title << std::endl;

    if (!config.results.should_compile) {
        error("cannot generate gold file for test `", config.title, "' which is not supposed to be ran.");
        return;
    }

    std::filesystem::create_directories(gold_dir);
    int build_res = run_cmd("../build/s3c", "-o", exec, src, config.compiler.flags, config.compiler.ldflags);
    if (build_res != 0 && config.results.should_compile) {
        error("failed to compile test '", config.title, "'.");
        return;
    }
    run_cmd(exec, ">", gold);
    std::cout << config.title << ": gold file generated successfuly." << std::endl;
}

void update_all_gold_files() {
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

    // parse config

    // list configs
    // list_tests("cnd");
    // update_gold_files("config/cnd.conf");
    assert(run_test("config/cnd.conf"));

    return 0;
}
