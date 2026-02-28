#include "compiler.hpp"
#include "compiler/tools.hpp"
#include "x86_64_gnu_linux.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace compiler {

std::string asm_addr(Address const &result) {
    switch (result.addressing_mode) {
    case AddressingMode::ImmediateValue:
        return result.immediate_value;
        break;
    case AddressingMode::Register:
        return result.register_name;
        break;
    case AddressingMode::RegisterIndirect:
        return "[" + result.register_name + "]";
        break;
    case AddressingMode::Index:
        std::cerr << "unimplemented " << __FILE__ << " " << __LINE__
                  << std::endl;
        break;
    case AddressingMode::Based:
        if (result.offset < 0) {
            return "[" + result.register_name + std::to_string(result.offset) +
                   "]";
        } else {
            return "[" + result.register_name + "+" +
                   std::to_string(result.offset) + "]";
        }
        break;
    }
    return result.register_name;
}

void asm_addr_immediate_value(CompilerState *state, std::string value,
                              Type *type) {
    // TODO: size?
    state->last_expr_addr.addressing_mode = AddressingMode::ImmediateValue;
    state->last_expr_addr.immediate_value = value;
    state->last_expr_addr.type = type;
}

void asm_addr_register(CompilerState *state, std::string register_name,
                       Type *type) {
    // TODO: size?
    state->last_expr_addr.addressing_mode = AddressingMode::Register;
    state->last_expr_addr.register_name = register_name;
    state->last_expr_addr.type = type;
}

void asm_addr_register_indirect(CompilerState *state, std::string register_name,
                                Type *type) {
    // TODO: size?
    state->last_expr_addr.addressing_mode = AddressingMode::RegisterIndirect;
    state->last_expr_addr.register_name = register_name;
    state->last_expr_addr.type = type;
}

void asm_addr_based(CompilerState *state, std::string base_name, int offset,
                    Type *type) {
    // TODO: size?
    state->last_expr_addr.addressing_mode = AddressingMode::Based;
    state->last_expr_addr.register_name = base_name;
    state->last_expr_addr.offset = offset;
    state->last_expr_addr.type = type;
}

// TODO: return a bool
void compile_x86_64(std::string const &filename, CompilerState *state,
                    Platform platform, Program const &program) {
    switch (platform) {
    case Platform::GNULinux:
        x86_64::gnu_linux::compile(state, program);
        break;
    }
    asm_dump(state->code, filename);
}

void compile(std::string const &filename, Arch arch, Platform platform,
             Program const &program) {
    CompilerState state{
        .code = {},
        .curr_function_id = "",
        .variables_addresses = {},
        .frame_offset = 0,
        .last_expr_addr = {},
    };
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
        if (!instruction.comment.empty()) {
            fs << " # " << instruction.comment;
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
    std::ofstream fs(filename);

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
    code.instructions.push_back(Instruction{instruction, arg1, arg2, ""});
}

void asm_comment_last_instruction(Asm &code, std::string const &comment) {
    code.instructions.back().comment = comment;
}

void asm_add_comment_line(Asm &code, std::string const &comment) {
    code.instructions.push_back(Instruction{"# " + comment, "", "", ""});
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

void allocate_stack_variable(CompilerState *state, std::string const &id,
                             size_t size, Type *type,
                             std::string const &base_name) {
    auto it = state->variables_addresses.find(id);

    if (it == state->variables_addresses.end()) {
        state->variables_addresses[id] = {};
    }
    Address addr;
    addr.addressing_mode = AddressingMode::Based;
    addr.offset = -state->frame_offset;
    addr.size = size;
    addr.type = type;
    addr.register_name = base_name;
    state->variables_addresses[id].push(addr);
    state->frame_offset += (int)size;
}

Address get_address(CompilerState *state, std::string const &id) {
    auto it = state->variables_addresses.find(id);

    if (it == state->variables_addresses.end()) {
        std::cerr << "error: unkown variable" << std::endl;
        return {};
    }
    return it->second.top();
}

} // end namespace compiler
