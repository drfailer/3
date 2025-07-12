#ifndef COMPILER_COMPILER
#define COMPILER_COMPILER
#include "program.hpp"
#include <map>
#include <stack>
#include <string>
#include <vector>

namespace compiler {

struct Instruction {
    std::string instruction;
    std::string arg1;
    std::string arg2;
    std::string comment;
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

enum class AddressingMode {
    ImmediateValue,
    Register,
    RegisterIndirect,
    Index,
    Based,
};

struct Address {
    // TODO: could be a label in the data section as well
    // TODO: an enum would be greate
    AddressingMode addressing_mode;
    std::string immediate_value;
    std::string register_name;
    std::string index;
    signed int offset;
    size_t size;
    type::Type *type;
};

struct CompilerState {
    Asm code;
    std::string curr_function_id;
    std::map<std::string, std::stack<Address>> variables_addresses;
    signed int frame_offset;
    Address last_expr_addr;
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

std::string asm_addr(Address const &result);
void asm_addr_immediate_value(CompilerState *state, std::string value,
                              type::Type *type);
void asm_addr_register(CompilerState *state, std::string register_name,
                       type::Type *type);
void asm_addr_register_indirect(CompilerState *state, std::string register_name,
                                type::Type *type);
void asm_addr_based(CompilerState *state, std::string base_name, int offset,
                    type::Type *type);

void asm_dump(Asm const &code, std::string const &filename);
void asm_add_global_symbol(Asm &code, std::string const &name);
void asm_add_label(Asm &code, std::string const &label);
void asm_add_instruction(Asm &code, std::string const &instruction,
                         std::string const &arg1 = "",
                         std::string const &arg2 = "");
void asm_comment_last_instruction(Asm &code, std::string const &comment);
void asm_add_data(Asm &code, std::string const &name, std::string const &type,
                  std::string const &value);
std::string asm_create_data_id(Asm const &code, std::string const &name);

void allocate_stack_variable(CompilerState *state, std::string const &id,
                             size_t size, type::Type *type,
                             std::string const &base_name);
Address get_address(CompilerState *state, std::string const &id);

} // end namespace compiler

#endif
