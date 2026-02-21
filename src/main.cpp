#include "compiler/compiler.hpp"
#include "compiler/tools.hpp"
#include "parser/lexer.hpp"
#include "parser/parser.hpp"
#include "preprocessor/preprocessor.hpp"
#include "s3c.hpp"
#include "checks.hpp"
#include "tools/messages.hpp"
#include "tools/string.hpp"
#include <filesystem>
#include <cassert>
#define PREPROCESSOR_OUTPUT_FILE "__main_pp__"

struct Options {
    std::string input_file; // TODO: will be changed to a list
    std::string output_file;
    std::string build_directory_name;
    enum {
        GenerateExecutable,
        GenerateAssembly,
    } generate_option;
    std::vector<std::string> linker_options;
};

Options parse_args(std::vector<std::string> const &args) {
    Options opts = {
        .input_file = "",
        .output_file = "bin",
        .build_directory_name = "build/",
        .generate_option = Options::GenerateExecutable,
        .linker_options = {},
    };
    auto arg = args.begin();

    while (arg != args.end()) {
        if (*arg == "-o" || *arg == "--output") {
            arg++;
            if (arg == args.end()) {
                std::cerr << "error: expected file name after " << *arg << "."
                          << std::endl;
                exit(1);
            }
            opts.output_file = *arg;
        } else if (starts_with(*arg, "--output=")) {
            opts.output_file = arg->substr(9);
        } else if (*arg == "--build-dir") {
            arg++;
            if (arg == args.end()) {
                std::cerr << "error: expected directory name after " << *arg
                          << "." << std::endl;
                exit(1);
            }
            opts.build_directory_name = *arg;
        } else if (starts_with(*arg, "--build-dir=")) {
            opts.build_directory_name = arg->substr(12);
        } else if (starts_with(*arg, "-L")) {
            opts.linker_options.push_back(*arg);
            if (arg->size() == 2) {
                arg++;
                if (arg == args.end()) {
                    std::cerr << "error: expected file name after " << *arg
                              << "." << std::endl;
                    exit(1);
                }
                opts.linker_options.push_back(*arg);
            }
        } else if (starts_with(*arg, "-l")) {
            opts.linker_options.push_back(*arg);
        } else if (starts_with(*arg, "-rpath=")) {
            opts.linker_options.push_back(*arg);
        } else if (*arg == "-S") {
            opts.generate_option = Options::GenerateAssembly;
        } else if ((*arg)[0] == '-') {
            std::cerr << "error: unknown option " << *arg << "." << std::endl;
            exit(1);
        } else {
            opts.input_file = *arg;
        }
        arg++;
    }
    if (opts.input_file.empty()) {
        std::cerr << "error: no input file." << std::endl;
        exit(1);
    }
    if (opts.output_file.empty()) {
        if (opts.generate_option == Options::GenerateExecutable) {
            opts.output_file =
                opts.input_file.substr(0, opts.input_file.size() - 2);
        } else {
            opts.output_file =
                opts.input_file.substr(0, opts.input_file.size() - 1) + "asm";
        }
    }
    return opts;
}

/* add execution rights to the result file */
void make_file_executable(std::string const &fielname) {
    std::filesystem::permissions(fielname,
                                 std::filesystem::perms::owner_exec |
                                     std::filesystem::perms::group_exec |
                                     std::filesystem::perms::others_exec,
                                 std::filesystem::perm_options::add);
}

void make_directory(std::string const &directory_name) {
    if (!std::filesystem::exists(directory_name)) {
        std::filesystem::create_directory(directory_name);
    }
}

bool preprocess(Options const &opts) {
    Preprocessor pp(opts.build_directory_name + PREPROCESSOR_OUTPUT_FILE);

    try {
        pp.process(opts.input_file); // launch the preprocessor
    } catch (std::logic_error &e) {
        msg::error(e.what());
        return false;
    }
    return true;
}

bool parse(Options const &opts, s3c::State *state) {
    // BUG: when declaring a string here, its destructor ends up crashing???
    // std::string pp_file = opts.build_directory_name + PREPROCESSOR_OUTPUT_FILE;
    std::ifstream is(opts.build_directory_name + PREPROCESSOR_OUTPUT_FILE, std::ios::in);
    assert(is.good());
    assert(state != nullptr);
    parser::Scanner scanner(is, std::cerr);
    parser::Parser parser(scanner, state);
    int err = parser.parse();
    bool result = !(err || state->status);
    return result;
}

bool compile(Options const &opts) {
    s3c::State *state = s3c::state_create();

    make_directory(opts.build_directory_name);

    if (!preprocess(opts)) {
        return false;
    }

    if (!parse(opts, state)) {
        return false;
    }

    CheckState check_state = {"", &state->type_pool, state->allocator};
    if (!check(&check_state, state->program, state->symtable)) {
        return false;
    }

    // look for main
    if (!try_verify_main_type(state)) {
        return false;
    }

    if (opts.generate_option == Options::GenerateAssembly) {
        auto base_name = compiler::base_name(opts.input_file);
        auto asm_file = compiler::asm_filename(base_name);
        compiler::compile(asm_file, compiler::Arch::X86_64,
                          compiler::Platform::GNULinux,
                          Program{state->program, state->symtable});
    } else {
        auto base_name =
            opts.build_directory_name + compiler::base_name(opts.input_file);
        auto asm_file = compiler::asm_filename(base_name);
        auto obj_file = compiler::object_filename(base_name);

        compiler::compile(asm_file, compiler::Arch::X86_64,
                          compiler::Platform::GNULinux,
                          Program{state->program, state->symtable});
        int assembler_success = compiler::run_cmd("as", "-g", "-msyntax=intel",
                                                  asm_file, "-o", obj_file);
        if (assembler_success == 0) {
            compiler::run_cmd("ld", "-o", opts.output_file, obj_file, "-lc",
                              "-dynamic-linker", "/lib64/ld-linux-x86-64.so.2",
                              opts.linker_options);
        }
    }

    state_destroy(state);
    return true;
}

int main(int argc, char **argv) {
    std::vector<std::string> args;

    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    Options opts = parse_args(args);

    if (!compile(opts)) {
        std::cerr << "Compilation failed!" << std::endl;
        return 1;
    }
    return 0;
}
