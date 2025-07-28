#include "compiler/compiler.hpp"
#include "parser/lexer.hpp"
#include "parser/parser.hpp"
#include "preprocessor/preprocessor.hpp"
#include "s3c.hpp"
#include "tools/messages.hpp"
#include <filesystem>
#define PREPROCESSOR_OUTPUT_FILE "__main_pp.prog__"
#define REMOVE_PREPROCESSOR_FILE false

/* Run interactive parser. It was used during the beginning of the project. */
void cli() {
    s3c::State *state = s3c::state_create();
    parser::Scanner scanner{std::cin, std::cerr};
    parser::Parser parser{&scanner, state};
    s3c::enter_scope(state);
    parser.parse();
    // TODO s3c.errorsManager().report();
    // if (!s3c.errorsManager().getErrors()) {
    //     TODO s3c.programBuilder().display();
    // }
    delete state;
}

/* add execution rights to the result file */
void makeExecutable(std::string file) {
    std::filesystem::permissions(file,
                                 std::filesystem::perms::owner_exec |
                                     std::filesystem::perms::group_exec |
                                     std::filesystem::perms::others_exec,
                                 std::filesystem::perm_options::add);
}

bool compile(std::string filename, std::string outputName) {
    s3c::State *state = s3c::state_create();
    Preprocessor pp(PREPROCESSOR_OUTPUT_FILE);

    // s3c::enter_file(state, filename);
    // s3c::enter_scope(state);

    try {
        pp.process(filename); // launch the preprocessor
    } catch (std::logic_error &e) {
        msg::error(e.what());
        return false;
    }

    // open and parse the file
    std::ifstream is(PREPROCESSOR_OUTPUT_FILE,
                     std::ios::in); // parse the preprocessed file
    parser::Scanner scanner{is, std::cerr};
    parser::Parser parser{&scanner, state};
    int err = parser.parse();

    if (err || state->status) {
        return false;
    }

    if (!s3c::post_process(state)) {
        return false;
    }

    // look for main
    if (!try_verify_main_type(state)) {
        return false;
    }

    // if no errors, transpile the file
    // TODO: use the status from the state
    // if (!s3c.errorsManager().getErrors()) {
    compiler::compile(filename, compiler::Arch::X86_64,
                      compiler::Platform::GNULinux,
                      Program{state->program, state->scopes.global});
    // TODO: check compile status before running ld
    std::string ld_cmd = "ld -o " + outputName + " " +
                         compiler::object_filename(filename) +
                         " -lc -dynamic-linker /lib64/ld-linux-x86-64.so.2";
    std::cout << "running: " << ld_cmd << std::endl;
    system(ld_cmd.c_str());
    // }

    // remove the preprocessor output file
    if (REMOVE_PREPROCESSOR_FILE) {
        std::filesystem::remove(PREPROCESSOR_OUTPUT_FILE);
    }
    delete state;
    return true;
}

int main(int argc, char **argv) {
    if (argc == 2) {
        if (!compile(argv[1], "bin")) {
            std::cerr << "Compilation failed!" << std::endl;
            return 1;
        }
    } else {
        cli();
    }
    return 0;
}
