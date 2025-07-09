#include "x86_64_gnu_linux.hpp"
#include "compiler/compiler.hpp"
#include "compiler/tools.hpp"
#include "type/type.hpp"
#include <iostream>
#include <sstream>

/*
 * Convetions:
 * - the result of evaluated expressions are stored in rax
 */

namespace compiler {

namespace x86_64 {

namespace gnu_linux {

void compile_node(CompilerState *state, node::Node *node, SymbolTable *scope);

/*
 * When this function is called, rax will always contain the value
 */
void compile_value(CompilerState *state, node::Value *node) {
    switch (node->kind) {
    case node::ValueKind::Character:
        asm_add_instruction(state->code, "mov", "al",
                            std::to_string(node->value.character));
        asm_addr_register(state, "al");
        break;
    case node::ValueKind::Integer:
        asm_add_instruction(state->code, "mov", "rax",
                            std::to_string(node->value.integer));
        asm_addr_register(state, "rax");
        break;
    case node::ValueKind::Real:
        // TODO: learn how to use floats properly :D
        asm_add_instruction(state->code, "mov", "rax",
                            std::to_string(node->value.real));
        asm_addr_register(state, "rax");
        break;
    case node::ValueKind::String: {
        std::string label = asm_create_data_id(state->code, "value_");
        asm_add_data(state->code, label, ".string", node->value.string);
        asm_add_instruction(state->code, "lea", "rax", label);
        asm_addr_register(state, "rax");
    } break;
    }
}

void compile_variable_definition(CompilerState *state,
                                 node::VariableDefinition *node,
                                 SymbolTable *scope) {
    auto type = lookup(scope, node->name)->type;
    auto size = type::get_type_size(type);

    allocate_stack_variable(state, node->name, size);
    asm_add_instruction(state->code, "sub", "rsp", std::to_string(size));
    asm_comment_last_instruction(state->code, node->name);
}

void compile_assignement(CompilerState *state, node::Assignment *node,
                         SymbolTable *scope) {
    ExpressionAddr target;

    compile_node(state, node->target, scope);
    target = state->addr;

    if (target.addressing_mode == AddressingMode::RegisterIndirect) {
        asm_add_instruction(state->code, "push", target.register_name);
        compile_node(state, node->value, scope);
        asm_add_instruction(state->code, "pop", "rdx");
        asm_add_instruction(state->code, "mov", "[rdx]", asm_addr(state->addr));
    } else if (target.addressing_mode == AddressingMode::Based) {
        compile_node(state, node->value, scope);
        asm_add_instruction(state->code, "mov", asm_addr(target), "rax");
    } else {
        std::cerr << "error: invalide target." << std::endl;
    }
}

void compile_index_expression(CompilerState *state, node::IndexExpression *node,
                              SymbolTable *scope) {
    // TODO: handle float cases
    compile_node(state, node->index, scope); // result on rax
    // TODO: does gas supports the fancy intel syntax?
    asm_add_instruction(state->code, "lea", "rdx", asm_addr(state->addr));
    asm_add_instruction(state->code, "add", "rax", "rdx");
    asm_addr_register_indirect(state, "rax");
}

void compile_variable_reference(CompilerState *state,
                                node::VariableReference *node,
                                SymbolTable *scope) {
    // auto type = lookup(scope, node->name)->type;
    auto addr = get_stack_address(state, node->name);
    asm_addr_based(state, "rbp", addr.offset);
    // asm_add_instruction(state->code, "lea", "rax", "[" + dest + "]");
}

void compile_arithmetic_operation(CompilerState *state,
                                  node::ArithmeticOperation *node,
                                  SymbolTable *scope) {
    // TODO: use different operations depending on the operand type
    //       use xmm registers add addsd, ... for floats
    // TODO: compile function should return a result that will contain where the
    // result of the instruction/operations is store (rax, address on stack,
    // ...)
    // TODO: compile functions should have configuration to specidiy where the
    // result should go.
    // TODO: know when registers contain addresses or immediate values
    // (dereference or not dereference?)
    switch (node->kind) {
    case node::ArithmeticOperationKind::Add:
        compile_node(state, node->rhs, scope);
        asm_add_instruction(state->code, "push", asm_addr(state->addr));
        compile_node(state, node->lhs, scope);
        if (!(state->addr.addressing_mode == AddressingMode::Register &&
              state->addr.register_name == "rax")) {
            asm_add_instruction(state->code, "mov", "rax",
                                asm_addr(state->addr));
        }
        asm_add_instruction(state->code, "pop", "rdx");
        asm_add_instruction(state->code, "add", "rax", "rdx");
        break;
    case node::ArithmeticOperationKind::Sub:
        compile_node(state, node->rhs, scope);
        asm_add_instruction(state->code, "push", asm_addr(state->addr));
        compile_node(state, node->lhs, scope);
        if (!(state->addr.addressing_mode == AddressingMode::Register &&
              state->addr.register_name == "rax")) {
            asm_add_instruction(state->code, "mov", "rax",
                                asm_addr(state->addr));
        }
        asm_add_instruction(state->code, "pop", "rdx");
        asm_add_instruction(state->code, "sub", "rax", "rdx");
        break;
    case node::ArithmeticOperationKind::Mul:
        compile_node(state, node->rhs, scope);
        asm_add_instruction(state->code, "push", asm_addr(state->addr));
        compile_node(state, node->lhs, scope);
        asm_add_instruction(state->code, "pop", "rdx");
        // note: in 64 bits mode imul's 2 operands should be 32 bits long, and
        // the result is 64 bits.
        if (!(state->addr.addressing_mode == AddressingMode::Register &&
              state->addr.register_name == "rax")) {
            asm_add_instruction(state->code, "mov", "rax",
                                asm_addr(state->addr));
        }
        asm_add_instruction(state->code, "imul", "eax", "edx");
        break;
    case node::ArithmeticOperationKind::Div:
        // throw std::logic_error("div doesn't work");
        compile_node(state, node->rhs, scope);
        asm_add_instruction(state->code, "push", asm_addr(state->addr));
        compile_node(state, node->lhs, scope);
        asm_add_instruction(state->code, "pop", "rdi");
        if (!(state->addr.addressing_mode == AddressingMode::Register &&
              state->addr.register_name == "rax")) {
            asm_add_instruction(state->code, "mov", "rax",
                                asm_addr(state->addr));
        }
        // set rdx to 0 before calling idiv !
        asm_add_instruction(state->code, "xor", "rdx", "rdx");
        asm_add_instruction(state->code, "idiv", "rdi");
        break;
    }
    asm_addr_register(state, "rax");
}

#define LABEL(node, suffix) ptr_to_string(node) + suffix
#define TRUE_LABEL(node) LABEL(node, "_true")
#define FALSE_LABEL(node) LABEL(node, "_false")

void compile_cmp(CompilerState *state, node::BooleanOperation *node,
                 SymbolTable *scope, std::string const &jmp) {
    compile_node(state, node->lhs, scope);
    asm_add_instruction(state->code, "push", "rax");
    compile_node(state, node->rhs, scope);
    asm_add_instruction(state->code, "pop", "rdx");
    asm_add_instruction(state->code, "cmp", "rax", "rdx");
    asm_add_instruction(state->code, jmp, TRUE_LABEL(node));
    asm_add_instruction(state->code, "jmp", FALSE_LABEL(node));
}

void compile_boolean_operation(CompilerState *state,
                               node::BooleanOperation *node,
                               SymbolTable *scope) {
    switch (node->kind) {
    case node::BooleanOperationKind::And:
        compile_node(state, node->lhs, scope);
        asm_add_label(state->code, TRUE_LABEL(node->lhs));
        compile_node(state, node->rhs, scope);
        asm_add_label(state->code, TRUE_LABEL(node->rhs));
        asm_add_instruction(state->code, "jmp", TRUE_LABEL(node));
        asm_add_label(state->code, FALSE_LABEL(node->lhs));
        asm_add_label(state->code, FALSE_LABEL(node->rhs));
        asm_add_instruction(state->code, "jmp", FALSE_LABEL(node));
        break;
    case node::BooleanOperationKind::Lor:
        compile_node(state, node->lhs, scope);
        asm_add_label(state->code, FALSE_LABEL(node->lhs));
        compile_node(state, node->rhs, scope);
        asm_add_label(state->code, FALSE_LABEL(node->rhs));
        asm_add_instruction(state->code, "jmp", FALSE_LABEL(node));
        asm_add_label(state->code, TRUE_LABEL(node->lhs));
        asm_add_label(state->code, TRUE_LABEL(node->rhs));
        asm_add_instruction(state->code, "jmp", TRUE_LABEL(node));
        break;
    case node::BooleanOperationKind::Xor:
        // evaluate lhs
        compile_node(state, node->lhs, scope);
        asm_add_label(state->code, TRUE_LABEL(node->lhs));
        asm_add_instruction(state->code, "push", "1");
        asm_add_instruction(state->code, "jmp", LABEL(node, "_rhs"));
        asm_add_label(state->code, FALSE_LABEL(node->lhs));
        asm_add_instruction(state->code, "push", "0");
        // evaluate rhs
        asm_add_label(state->code, LABEL(node, "_rhs"));
        compile_node(state, node->rhs, scope);
        asm_add_label(state->code, TRUE_LABEL(node->rhs));
        asm_add_instruction(state->code, "mov", "rax", "1");
        asm_add_instruction(state->code, "jmp", LABEL(node, "_xor"));
        asm_add_label(state->code, FALSE_LABEL(node->rhs));
        asm_add_instruction(state->code, "mov", "rax", "0");
        // xor
        asm_add_label(state->code, LABEL(node, "_xor"));
        asm_add_instruction(state->code, "pop", "rdx");
        asm_add_instruction(state->code, "xor", "rax", "rdx");
        asm_add_instruction(state->code, "jnz", TRUE_LABEL(node));
        asm_add_instruction(state->code, "jmp", FALSE_LABEL(node));
        break;
    case node::BooleanOperationKind::Not:
        compile_node(state, node->lhs, scope);
        asm_add_label(state->code, FALSE_LABEL(node->lhs));
        asm_add_instruction(state->code, "jmp", FALSE_LABEL(node));
        asm_add_label(state->code, TRUE_LABEL(node->lhs));
        asm_add_instruction(state->code, "jmp", TRUE_LABEL(node));
        break;
        // TODO: be carefull with unsigned in the future
    case node::BooleanOperationKind::Eql:
        compile_cmp(state, node, scope, "je");
        break;
    case node::BooleanOperationKind::Inf:
        compile_cmp(state, node, scope, "ji");
        break;
    case node::BooleanOperationKind::Sup:
        compile_cmp(state, node, scope, "js");
        break;
    case node::BooleanOperationKind::Ieq:
        compile_cmp(state, node, scope, "jie");
        break;
    case node::BooleanOperationKind::Seq:
        compile_cmp(state, node, scope, "jse");
        break;
    }
}

// TODO: builtin function should not be compiled but linked !
void compile_builtin_function(CompilerState *state, node::BuiltinFunction *node,
                              SymbolTable *scope) {
    std::cout << "comiple builtin function" << std::endl;
    switch (node->kind) {
    case node::BuiltinFunctionKind::Shw: {
        // TODO: for now we only support text
        std::string msg_id = asm_create_data_id(state->code, "shw_msg");
        std::string msg = node->argument->value.value->value.string;
        std::string msg_len = std::to_string(get_compiled_string_size(msg));

        asm_add_data(state->code, msg_id, ".string", "\"" + msg + "\"");
        // save registers?
        asm_add_instruction(state->code, "mov", "rax", "1");
        asm_add_instruction(state->code, "mov", "rdi", "1");
        asm_add_instruction(state->code, "lea", "rsi", msg_id);
        asm_add_instruction(state->code, "mov", "rdx", msg_len);
        asm_add_instruction(state->code, "syscall");
    } break;
    case node::BuiltinFunctionKind::Ipt:
        // TODO
        break;
    }
}

void compile_function_call(CompilerState *state, node::FunctionCall *node,
                           SymbolTable *scope) {
    // TODO
}

void compile_cnd_stmt(CompilerState *state, node::CndStmt *node,
                      SymbolTable *scope) {
    // TODO
}

void compile_for_stmt(CompilerState *state, node::ForStmt *node,
                      SymbolTable *scope) {
    // TODO
}

void compile_whl_stmt(CompilerState *state, node::WhlStmt *node,
                      SymbolTable *scope) {
    // TODO
}

void compile_ret_stmt(CompilerState *state, node::RetStmt *node,
                      SymbolTable *scope) {
    compile_node(state, node->expression, scope);
    if (node->expression->kind != node::NodeKind::Value) {
        asm_add_instruction(state->code, "mov", "rax", asm_addr(state->addr));
    }
    // TODO: this is not required if the return is at the end of the block
    asm_add_instruction(state->code, "jmp",
                        "epilogue_" + state->curr_function_id);
}

void compile_block(CompilerState *state, node::Block *node,
                   SymbolTable *scope) {
    for (auto instruction : node->nodes) {
        compile_node(state, instruction, scope);
    }
}

void compile_function_definition(CompilerState *state,
                                 node::FunctionDefinition *node,
                                 SymbolTable *scope) {
    SymbolTable *function_scope = scope->symbols[node->name].scope;
    state->curr_function_id = node->name;

    asm_add_label(state->code, node->name);
    asm_add_instruction(state->code, "push", "rbp");
    asm_add_instruction(state->code, "mov", "rbp", "rsp");

    // TODO: add argument locations

    // TODO: we want to go down the scope
    compile_block(state, node->body, function_scope);

    asm_add_label(state->code, "epilogue_" + node->name);
    asm_add_instruction(state->code, "mov", "rsp", "rbp");
    asm_add_instruction(state->code, "pop", "rbp");
    asm_add_instruction(state->code, "ret");
    state->frame_offset = 0;
}

void compile_function_declaration(CompilerState *state,
                                  node::FunctionDeclaration *node,
                                  SymbolTable *scope) {
    // TODO: add global symbol (the function is a foreign symbol for which the
    // address will be resolved at link time)
}

void compile_node(CompilerState *state, node::Node *node, SymbolTable *scope) {
    std::cout << "compile node " << (int)node->kind << std::endl;
    switch (node->kind) {
    case node::NodeKind::Value:
        compile_value(state, node->value.value);
        break;
    case node::NodeKind::VariableDefinition:
        compile_variable_definition(state, node->value.variable_definition,
                                    scope);
        break;
    case node::NodeKind::VariableReference:
        compile_variable_reference(state, node->value.variable_reference,
                                   scope);
        break;
    case node::NodeKind::Assignment:
        compile_assignement(state, node->value.assignment, scope);
        break;
    case node::NodeKind::IndexExpression:
        compile_index_expression(state, node->value.index_expression, scope);
        break;
    case node::NodeKind::FunctionDefinition:
        compile_function_definition(state, node->value.function_definition,
                                    scope);
        break;
    case node::NodeKind::FunctionDeclaration:
        compile_function_declaration(state, node->value.function_declaration,
                                     scope);
        break;
    case node::NodeKind::FunctionCall:
        compile_function_call(state, node->value.function_call, scope);
        break;
    case node::NodeKind::CndStmt:
        compile_cnd_stmt(state, node->value.cnd_stmt, scope);
        break;
    case node::NodeKind::WhlStmt:
        compile_whl_stmt(state, node->value.whl_stmt, scope);
        break;
    case node::NodeKind::ForStmt:
        compile_for_stmt(state, node->value.for_stmt, scope);
        break;
    case node::NodeKind::RetStmt:
        compile_ret_stmt(state, node->value.ret_stmt, scope);
        break;
    case node::NodeKind::Block:
        // TODO: this one might be useless
        compile_block(state, node->value.block, scope);
        break;
    case node::NodeKind::ArithmeticOperation:
        compile_arithmetic_operation(state, node->value.arithmetic_operation,
                                     scope);
        break;
    case node::NodeKind::BooleanOperation:
        compile_boolean_operation(state, node->value.boolean_operation, scope);
        break;
    case node::NodeKind::BuiltinFunction:
        compile_builtin_function(state, node->value.builtin_function, scope);
        break;
    }
}

void make_start(CompilerState *state) {
    asm_add_global_symbol(state->code, "_start");
    asm_add_label(state->code, "_start");
    asm_add_instruction(state->code, "call", "main");
    asm_add_instruction(state->code, "mov", "rdi", "rax");
    asm_add_instruction(state->code, "mov", "rax", "60");
    asm_add_instruction(state->code, "syscall");
}

void compile(CompilerState *state, Program const &program) {
    for (auto node : program.code) {
        compile_node(state, node, program.scope);
    }
    if (lookup(program.scope, "main")) {
        make_start(state);
    }
}

} // namespace gnu_linux

} // namespace x86_64

} // end namespace compiler
