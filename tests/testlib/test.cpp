#include "test.hpp"
#include "config.hpp"
#include "cmd.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cassert>
#include <fstream>

template <typename ...Ts>
void error(Ts ...args) {
    std::cerr << "error: ";
    (std::cerr << ... << args) << std::endl;
}

static bool files_equal(std::string const& gold_path, std::string const& out_path) {
  std::ifstream fgold(gold_path);
  std::ifstream fout(out_path);
  assert(fgold.good());
  assert(fout.good());
  return std::equal(
          std::istreambuf_iterator<char>(fgold.rdbuf()),
          std::istreambuf_iterator<char>(),
          std::istreambuf_iterator<char>(fout.rdbuf()));
}

void list_tests(std::string const &pattern) {
    auto paths = get_config_files(CONFIG_DIR, pattern);

    for (auto path : paths) {
        auto config = parse_config_file(path);
        std::cout << config.title << "(" << config.category << "): " << config.description << std::endl;
    }
}

static bool run_test(TestConfig const &config) {
    std::string base_name = config.files.src.substr(0, config.files.src.size() - 2);
    std::string out_dir  = OUT_DIR + config.files.dir;
    std::string src_dir  = SRC_DIR + config.files.dir;
    std::string gold_dir  = GOLD_DIR + config.files.dir;
    std::string exec  = out_dir + "/" + base_name;
    std::string comp  = out_dir + "/" + base_name + ".comp";
    std::string out  = out_dir + "/" + base_name + ".out";
    std::string src  = src_dir + "/" + config.files.src;
    std::string gold_comp  = gold_dir + "/" + base_name + ".comp";
    std::string gold_out  = gold_dir + "/" + base_name + ".out";

    std::filesystem::create_directories(out_dir);

    // build
    if (!std::filesystem::exists(gold_comp)) {
        error("no gold files found for test `", config.title, "'");
        return false;
    }
    int build_res = run_cmd("../build/s3c", "-o",
                            exec, src, config.compiler.flags, config.compiler.ldflags,
                            ">", comp);
    if (build_res != 0 && config.results.should_compile) {
        error("failed to compile test '", config.title, "'.");
        return false;
    }
    if (!files_equal(gold_comp, comp)) {
        error("compiler output missmatch for test `", config.title , "'.");
        return false;
    }

    // run
    if (!config.results.should_run) {
        return true;
    }
    if (!std::filesystem::exists(gold_out)) {
        error("no gold files found for test `", config.title, "'");
        return false;
    }
    int run_res = run_cmd(exec, ">", out);
    if (run_res != config.results.exit_code) {
        error("wrong exit code for program `", config.title, "', expected ",
              config.results.exit_code, " but found ", run_res, ".");
        return false;
    }
    if (!files_equal(gold_out, out)) {
        error("out file does not correspond to gold file for test `", config.title, "'.");
        return false;
    }
    return true;
}

void run_tests(std::string const& pattern) {
    auto config_paths = get_config_files(CONFIG_DIR, pattern);

    for (auto path : config_paths) {
        TestConfig config = parse_config_file(path);
        if (run_test(config)) {
            std::cout << "success: " << config.title << std::endl;
        } else {
            std::cout << "failure: " << config.title << std::endl;
        }
    }
}

static void update_test_gold_files(std::string const &path_to_config) {
    TestConfig config = parse_config_file(path_to_config);
    std::string base_name = config.files.src.substr(0, config.files.src.size() - 2);
    std::string src_dir  = SRC_DIR + config.files.dir;
    std::string out_dir  = OUT_DIR + config.files.dir;
    std::string gold_dir  = GOLD_DIR + config.files.dir;
    std::string exec  = out_dir + "/" + base_name;
    std::string src  = src_dir + "/" + config.files.src;
    std::string comp  = gold_dir + "/" + base_name + ".comp";
    std::string out  = gold_dir + "/" + base_name + ".out";

    if (!config.results.should_compile || !config.results.should_run) {
        return;
    }

    std::cout << "update gold file for test '" << config.title << std::endl;

    std::filesystem::create_directories(gold_dir);
    int build_res = run_cmd("../build/s3c", "-o",
                            exec, src, config.compiler.flags, config.compiler.ldflags,
                            ">", comp);
    if (build_res != 0 && config.results.should_compile) {
        error("failed to compile test '", config.title, "'.");
        return;
    }
    if (!config.results.should_run) {
        return;
    }
    int run_res = run_cmd(exec, ">", out);
    if (run_res != config.results.exit_code) {
        error("cannot update gold file due to missmatch return type. Found ",
              run_res, " bun expected ", config.results.exit_code);
    }
    std::cout << config.title << ": gold file generated successfuly." << std::endl;
}

void update_tests_gold_files(std::string const& pattern) {
    auto configs = get_config_files(CONFIG_DIR, pattern);

    for (auto config : configs) {
        update_test_gold_files(config);
    }
}
