#ifndef TESTLIB_TEST
#define TESTLIB_TEST
#include <string>

#define PATH_TO_S3C "../build/s3c"
#define CONFIG_DIR "config/"
#define OUT_DIR "out/"
#define SRC_DIR "src/"
#define GOLD_DIR "gold/"

void list_tests(std::string const &pattern = "");
void run_tests(std::string const& pattern = "");
void update_tests_gold_files(std::string const& pattern = "");

#endif
