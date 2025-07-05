#ifndef COMPILER_COMPILER
#define COMPILER_COMPILER
#include <map>
#include <stack>
#include <string>
#include <vector>
#include "program.hpp"

namespace compiler {

struct StackAddress {
    signed int offset;
    enum {
        StackPointer,
        BasePointer,
    } base;
};

struct Instruction {
    std::string instruction;
    std::string arg1;
    std::string arg2;
};

struct Data {
    std::string label;
    std::string type;
    std::string value;
};

struct Asm {
    std::vector<std::string> global_symbols;
    std::vector<Instruction> instructions;
    std::vector<Data> data;
};

struct CompilerState {
    Asm code;
    std::string curr_function_id;
    std::map<std::string, std::stack<StackAddress>> variables_addresses;
    signed int stack_offset;
};

enum class Arch {
    X86_64,
};

enum class Platform {
    GNULinux,
};

std::string object_filename(std::string const &filename);
std::string asm_filename(std::string const &filename);

void compile(std::string const &filename, Arch arch, Platform platform,
             Program const &program);

void asm_dump(Asm const &code, std::string const &filename);
void asm_add_global_symbol(Asm &code, std::string const &name);
void asm_add_label(Asm &code, std::string const &label);
void asm_add_instruction(Asm &code, std::string const &instruction,
                         std::string const &arg1 = "",
                         std::string const &arg2 = "");
void asm_add_data(Asm &code, std::string const &name, std::string const &type,
                  std::string const &value);
std::string asm_create_data_id(Asm const &code, std::string const &name);

void allocate_stack_variable(CompilerState *state, std::string const &id,
                             size_t size);
StackAddress get_stack_address(CompilerState *state, std::string const &id);

} // end namespace compiler

#endif
