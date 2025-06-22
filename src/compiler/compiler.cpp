#include "compiler.hpp"
#include "x86_64_gnu_linux.hpp"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace compiler {

std::string asm_filename(std::string const &filename) {
    std::string base_name = std::filesystem::path(filename).filename();
    return base_name + ".asm";
}

std::string object_filename(std::string const &filename) {
    std::string base_name = std::filesystem::path(filename).filename();
    return base_name + ".o";
}

void compile_x86_64(std::string const &filename, CompilerState *state, Platform platform,
                    Program const &program) {
    switch (platform) {
    case Platform::GNULinux:
        x86_64::gnu_linux::compile(state, program);
        break;
    }
    asm_dump(state->code, filename);
    // TODO: run the following command:
    // TODO: read as manual to change the output name
    std::string as_cmd = "as -msyntax=intel " + asm_filename(filename) +
                         " -o " + object_filename(filename);
    system(as_cmd.c_str());
    // ld -o output_name a.out
    // TODO: ld should be done afterward!
}

void compile(std::string const &filename, Arch arch, Platform platform,
             Program const &program) {
    CompilerState state;
    switch (arch) {
    case Arch::X86_64:
        compile_x86_64(filename, &state, platform, program);
        break;
    };
}

void asm_dump_global_symbols(Asm const &code, std::ofstream &fs) {
    for (auto sym : code.global_symbols) {
        fs << ".global " << sym << std::endl;
    }
}

void asm_dump_instructions(Asm const &code, std::ofstream &fs) {
    for (auto instruction : code.instructions) {
        fs << "    " << instruction.instruction;
        if (!instruction.arg1.empty()) {
            fs << " " << instruction.arg1;
        }
        if (!instruction.arg2.empty()) {
            fs << ", " << instruction.arg2;
        }
        fs << std::endl;
    }
}

void asm_dump_data(Asm const &code, std::ofstream &fs) {
    for (auto data : code.data) {
        fs << data.label << ": " << data.type << " " << data.value << std::endl;
    }
}

void asm_dump(Asm const &code, std::string const &filename) {
    std::ofstream fs(asm_filename(filename));

    fs << ".intel_syntax noprefix" << std::endl;
    fs << ".text" << std::endl;
    asm_dump_global_symbols(code, fs);
    asm_dump_instructions(code, fs);

    fs << ".data" << std::endl;
    asm_dump_data(code, fs);
}

void asm_add_global_symbol(Asm &code, std::string const &name) {
    code.global_symbols.push_back(name);
}

void asm_add_label(Asm &code, std::string const &label) {
    asm_add_instruction(code, label + ":");
}

void asm_add_instruction(Asm &code, std::string const &instruction,
                         std::string const &arg1, std::string const &arg2) {
    code.instructions.push_back(Instruction{instruction, arg1, arg2});
}

void asm_add_data(Asm &code, std::string const &name, std::string const &type,
                  std::string const &value) {
    code.data.push_back(Data{name, type, value});
}

std::string asm_create_data_id(Asm const &code, std::string const &name) {
    std::ostringstream oss;
    oss << name << code.data.size();
    return oss.str();
}

} // end namespace compiler
