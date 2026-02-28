#include "x86_64_gnu_linux.hpp"
#include "compiler/compiler.hpp"
#include "compiler/tools.hpp"
#include "scope.hpp"
#include "tools/string.hpp"
#include "../type.hpp"
#include <array>
#include <cassert>
#include <cstring>
#include <iostream>

/*
 * Conventions:
 * - the result of evaluated expressions are stored in rax
 */

namespace compiler {

namespace x86_64 {

namespace gnu_linux {

const std::array<std::string, 6> ARG_REGISTERS_INTEGER = {"rdi", "rsi", "rdx",
                                                          "rcx", "r8",  "r9"};

const std::array<std::string, 8> ARG_REGISTERS_FLOAT = {
    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"};

void asm_mov(CompilerState *state, std::string dest, std::string const &src) {
    if (dest == src) {
        return;
    }
    if (starts_with(src, "xmm") || starts_with(dest, "xmm")) {
        asm_add_instruction(state->code, "movsd", dest, src);
    } else {
        asm_add_instruction(state->code, "mov", dest, src);
    }
}

void asm_mov(CompilerState *state, Address const &dest_addr, std::string const &src) {
    asm_mov(state, asm_addr(dest_addr), src);
}

void mov_result_to_register(CompilerState *state, std::string const &register_name) {
    asm_mov(state, register_name, asm_addr(state->last_expr_addr));
}

Address create_tmp_str_value(CompilerState *state, Ast *value, Address addr) {
    std::string value_str(value->data.value.value.string.ptr);
    Address result_addr;

    result_addr.addressing_mode = AddressingMode::Register;
    result_addr.register_name = "rax";
    result_addr.type = addr.type;
    assert(addr.addressing_mode == AddressingMode::ImmediateValue);

    // nb bytes
    asm_add_instruction(state->code, "push", std::to_string(value_str.size()));
    // data ptr
    asm_add_instruction(state->code, "lea", "rax", asm_addr(addr));
    asm_add_instruction(state->code, "push", "rax");
    asm_add_instruction(state->code, "lea", "rax", "[rsp+8]");
    return result_addr;
}

void compile_block(CompilerState *state, Ast *ast, Scope *scope);

void compile_ast(CompilerState *state, Ast *ast, Scope *scope);

/*
 * When this function is called, rax will always contain the value
 */
void compile_value(CompilerState *state, Ast *ast, Scope *scope) {
    Value *value_ast = &ast->data.value;
    Type *type = scope->expr_types[ast];

    switch (value_ast->kind) {
    case ValueKind::Character:
        asm_mov(state, "al", std::to_string(value_ast->value.character));
        asm_addr_register(state, "al", type);
        break;
    case ValueKind::Integer:
        asm_mov(state, "rax", std::to_string(value_ast->value.integer));
        asm_addr_register(state, "rax", type);
        break;
    case ValueKind::Real: {
        auto flt_id = asm_create_data_id(state->code, "flt");
        asm_add_data(state->code, flt_id, ".double",
                     std::to_string(value_ast->value.real));
        asm_mov(state, "xmm0", flt_id);
        asm_addr_register(state, "xmm0", type);
    } break;
    case ValueKind::String: {
        std::string label = asm_create_data_id(state->code, "value_");
        asm_add_data(state->code, label, ".string", value_ast->value.string.ptr);
        asm_addr_immediate_value(state, label, type);
    } break;
    }
}

void compile_variable_definition(CompilerState *state, Ast *ast, Scope *scope) {
    VariableDefinition *var_ast = &ast->data.variable_definition;
    auto type = scope_lookup_symbol(scope, var_ast->name.ptr)->type;
    auto size = size_of(type);

    size += size % 16; // ensure alignment
    allocate_stack_variable(state, var_ast->name.ptr, size, type, "rbp");
    asm_add_instruction(state->code, "sub", "rsp", std::to_string(size));
    asm_comment_last_instruction(state->code, var_ast->name.ptr);
}

void compile_assignement(CompilerState *state, Ast *ast, Scope *scope) {
    Assignment *assignment_ast = &ast->data.assignment;
    Address target;
    auto target_type = scope->expr_types[assignment_ast->target];
    auto value_type = scope->expr_types[assignment_ast->value];

    compile_ast(state, assignment_ast->target, scope);
    target = state->last_expr_addr;

    if (target.addressing_mode == AddressingMode::RegisterIndirect) {
        asm_add_instruction(state->code, "push", target.register_name);
        compile_ast(state, assignment_ast->value, scope);
        asm_add_instruction(state->code, "pop", "rdx");
        if (is_flt(target_type)) {
            std::string value_addr = asm_addr(state->last_expr_addr);
            if (!is_flt(value_type)) {
                asm_add_instruction(state->code, "cvtsi2sd", "xmm0", value_addr);
                value_addr = "xmm0";
            }
            asm_mov(state, "[rdx]", value_addr);
        } else if (is_int(target_type)) {
            asm_mov(state, "[rdx]", asm_addr(state->last_expr_addr));
        } else if (is_str(target_type)) {
            throw std::logic_error(
                "error: str type does not support direct addressing mode.");
        } else {
            throw std::logic_error("unknown assignment target tyep: " +
                                   type_to_string(target_type));
        }
    } else if (target.addressing_mode == AddressingMode::Based) {
        compile_ast(state, assignment_ast->value, scope);
        if (is_flt(target_type)) {
            auto value_addr = asm_addr(state->last_expr_addr);
            if (!is_flt(value_type)) {
                asm_add_instruction(state->code, "cvtsi2sd", "xmm0", value_addr);
                value_addr = "xmm0";
            }
            asm_mov(state, target, value_addr);
        } else if (is_int(target_type)) {
            asm_mov(state, asm_addr(target), asm_addr(state->last_expr_addr));
        } else if (is_str(target_type)) {
            // str is store backward as followed on the stack:
            // [nb bytes] <- effective address of the string
            // [data ptr]
            if (assignment_ast->value->kind == AstKind::Value) {
                auto value_addr = state->last_expr_addr;
                std::string value_str(
                    assignment_ast->value->data.value.value.string.ptr);
                // nb bytes
                asm_mov(state, "rdx", std::to_string(value_str.size()));
                asm_mov(state, asm_addr(target), "rdx");
                // data ptr
                target.offset -= 8;
                asm_add_instruction(state->code, "lea", "rdx", asm_addr(value_addr));
                asm_mov(state, asm_addr(target), "rdx");
            } else {
                auto value_addr = state->last_expr_addr;
                if (value_addr.addressing_mode == AddressingMode::Based) {
                    // nb bytes
                    asm_mov(state, "rax", asm_addr(value_addr));
                    asm_mov(state, asm_addr(target), "rax");
                    // data ptr
                    target.offset -= 8;
                    value_addr.offset -= 8;
                    asm_add_instruction(state->code, "lea", "rax", asm_addr(value_addr));
                    asm_mov(state, asm_addr(target), "rax");
                } else {
                    // nb bytes
                    asm_mov(state, asm_addr(target), asm_addr(value_addr));
                    // data ptr
                    target.offset -= 8;
                    value_addr.offset -= 8;
                    asm_mov(state, asm_addr(target), asm_addr(value_addr));
                }
            }
        } else {
            throw std::logic_error("unknown assignment target tyep: " +
                                   type_to_string(target_type));
        }
    } else {
        std::cerr << "error: invalide target." << std::endl;
    }
}

void compile_index_expression(CompilerState *state, Ast *ast, Scope *scope) {
    IndexExpression *expr_ast = &ast->data.index_expression;
    auto type = scope->expr_types[ast];

    // TODO: make sure that this works with float arrays
    compile_ast(state, expr_ast->element, scope);
    auto element_addr = state->last_expr_addr;
    compile_ast(state, expr_ast->index, scope);
    auto index_result_addr = state->last_expr_addr;
    asm_add_instruction(state->code, "mov", "rdi", asm_addr(index_result_addr));
    asm_add_instruction(state->code, "lea", "rax", asm_addr(element_addr));
    asm_add_instruction(state->code, "add", "rax", "rdi");
    asm_addr_register_indirect(state, "rax", type);
}

void compile_variable_reference(CompilerState *state, Ast *ast, Scope *) {
    VariableReference *var_ref_ast = &ast->data.variable_reference;
    // TODO: the addressing mode might not be based all the time
    auto addr = get_address(state, var_ref_ast->name.ptr);
    asm_addr_based(state, "rbp", addr.offset, addr.type);
}

void compile_add_sub_int(CompilerState *state, ArithmeticOperation *ast,
                         Scope *scope, bool sub, Type *type) {
    compile_ast(state, ast->rhs, scope);
    asm_add_instruction(state->code, "push", asm_addr(state->last_expr_addr));
    compile_ast(state, ast->lhs, scope);
    mov_result_to_register(state, "rax");
    asm_add_instruction(state->code, "pop", "rdx");
    if (sub) {
        asm_add_instruction(state->code, "sub", "rax", "rdx");
    } else {
        asm_add_instruction(state->code, "add", "rax", "rdx");
    }
    asm_addr_register(state, "rax", type);
}

void compile_mul_int(CompilerState *state, ArithmeticOperation *ast, Scope *scope,
                     Type *type) {
    compile_ast(state, ast->rhs, scope);
    asm_add_instruction(state->code, "push", asm_addr(state->last_expr_addr));
    compile_ast(state, ast->lhs, scope);
    asm_add_instruction(state->code, "pop", "rdx");
    // note: in 64 bits mode imul's 2 operands should be 32 bits long, and
    // the result is 64 bits.
    mov_result_to_register(state, "rax");
    asm_add_instruction(state->code, "imul", "eax", "edx");
    asm_addr_register(state, "rax", type);
}

void compile_div_int(CompilerState *state, ArithmeticOperation *ast, Scope *scope,
                     Type *type) {
    compile_ast(state, ast->rhs, scope);
    asm_add_instruction(state->code, "push", asm_addr(state->last_expr_addr));
    compile_ast(state, ast->lhs, scope);
    asm_add_instruction(state->code, "pop", "rdi");
    mov_result_to_register(state, "rax");
    asm_add_instruction(state->code, "xor", "rdx", "rdx"); // zero rdx
    asm_add_instruction(state->code, "idiv", "rdi");
    asm_addr_register(state, "rax", type);
}

void compile_arithmetic_operation_flt(CompilerState *state,
                                      ArithmeticOperation *ast,
                                      Scope *scope, std::string const &op,
                                      Type *type) {
    compile_ast(state, ast->rhs, scope);
    asm_add_instruction(state->code, "sub", "rsp", "16");
    // implicit convertion to double
    if (is_flt(scope->expr_types[ast->rhs])) {
        asm_mov(state, "xmm0", asm_addr(state->last_expr_addr));
        asm_mov(state, "[rsp]", "xmm0");
    } else {
        asm_add_instruction(state->code, "cvtsi2sd", "xmm0", asm_addr(state->last_expr_addr));
        asm_mov(state, "[rsp]", "xmm0");
    }
    compile_ast(state, ast->lhs, scope);
    if (is_flt(scope->expr_types[ast->lhs])) {
        mov_result_to_register(state, "xmm0");
    } else {
        asm_add_instruction(state->code, "cvtsi2sd", "xmm0", asm_addr(state->last_expr_addr));
    }
    asm_mov(state, "xmm1", "[rsp]");
    asm_add_instruction(state->code, op, "xmm0", "xmm1");
    asm_addr_register(state, "xmm0", type);
    asm_add_instruction(state->code, "add", "rsp", "16");
}

void compile_arithmetic_operation(CompilerState *state, Ast *ast, Scope *scope) {
    ArithmeticOperation *op_ast = &ast->data.arithmetic_operation;
    Type *op_type = scope->expr_types[ast];
    switch (op_ast->kind) {
    case ArithmeticOperationKind::Add:
        if (is_flt(scope_lookup_expr_type(scope, ast))) {
            compile_arithmetic_operation_flt(state, op_ast, scope, "addsd",
                                             op_type);
        } else {
            compile_add_sub_int(state, op_ast, scope, false, op_type);
        }
        break;
    case ArithmeticOperationKind::Sub:
        if (is_flt(scope_lookup_expr_type(scope, ast))) {
            compile_arithmetic_operation_flt(state, op_ast, scope, "subsd",
                                             op_type);
        } else {
            compile_add_sub_int(state, op_ast, scope, true, op_type);
        }
        break;
    case ArithmeticOperationKind::Mul:
        if (is_flt(scope_lookup_expr_type(scope, ast))) {
            compile_arithmetic_operation_flt(state, op_ast, scope, "mulsd",
                                             op_type);
        } else {
            compile_mul_int(state, op_ast, scope, op_type);
        }
        break;
    case ArithmeticOperationKind::Div:
        if (is_flt(scope_lookup_expr_type(scope, ast))) {
            compile_arithmetic_operation_flt(state, op_ast, scope, "divsd",
                                             op_type);
        } else {
            compile_div_int(state, op_ast, scope, op_type);
        }
        break;
    }
}

#define LABEL(ast, suffix) "label_" + ptr_to_string(ast) + suffix
#define TRUE_LABEL(ast) LABEL(ast, "_true")
#define FALSE_LABEL(ast) LABEL(ast, "_false")
#define BEGIN_LABEL(ast) LABEL(ast, "_begin")
#define END_LABEL(ast) LABEL(ast, "_end")

void compile_cmp(CompilerState *state, Ast *ast, Scope *scope, std::string const &jmp) {
    Ast *lhs = ast->data.boolean_operation.lhs;
    Ast *rhs = ast->data.boolean_operation.rhs;

    compile_ast(state, lhs, scope);
    mov_result_to_register(state, "rax");
    asm_add_instruction(state->code, "push", "rax");
    compile_ast(state, rhs, scope);
    mov_result_to_register(state, "rax");
    asm_add_instruction(state->code, "pop", "rdx");
    asm_add_instruction(state->code, "cmp", "rdx", "rax");
    asm_add_instruction(state->code, jmp, TRUE_LABEL(ast));
    asm_add_instruction(state->code, "jmp", FALSE_LABEL(ast));
}

void compile_boolean_operation(CompilerState *state, Ast *ast, Scope *scope) {
    // TODO: float?
    BooleanOperation *op_ast = &ast->data.boolean_operation;
    switch (op_ast->kind) {
    case BooleanOperationKind::And:
        compile_ast(state, op_ast->lhs, scope);
        asm_add_label(state->code, TRUE_LABEL(op_ast->lhs));
        compile_ast(state, op_ast->rhs, scope);
        asm_add_label(state->code, TRUE_LABEL(op_ast->rhs));
        asm_add_instruction(state->code, "jmp", TRUE_LABEL(ast));
        asm_add_label(state->code, FALSE_LABEL(op_ast->lhs));
        asm_add_label(state->code, FALSE_LABEL(op_ast->rhs));
        asm_add_instruction(state->code, "jmp", FALSE_LABEL(ast));
        break;
    case BooleanOperationKind::Lor:
        compile_ast(state, op_ast->lhs, scope);
        asm_add_label(state->code, FALSE_LABEL(op_ast->lhs));
        compile_ast(state, op_ast->rhs, scope);
        asm_add_label(state->code, FALSE_LABEL(op_ast->rhs));
        asm_add_instruction(state->code, "jmp", FALSE_LABEL(ast));
        asm_add_label(state->code, TRUE_LABEL(op_ast->lhs));
        asm_add_label(state->code, TRUE_LABEL(op_ast->rhs));
        asm_add_instruction(state->code, "jmp", TRUE_LABEL(ast));
        break;
    case BooleanOperationKind::Xor:
        // evaluate lhs
        compile_ast(state, op_ast->lhs, scope);
        asm_add_label(state->code, TRUE_LABEL(op_ast->lhs));
        asm_add_instruction(state->code, "push", "1");
        asm_add_instruction(state->code, "jmp", LABEL(ast, "_rhs"));
        asm_add_label(state->code, FALSE_LABEL(op_ast->lhs));
        asm_add_instruction(state->code, "push", "0");
        // evaluate rhs
        asm_add_label(state->code, LABEL(ast, "_rhs"));
        compile_ast(state, op_ast->rhs, scope);
        asm_add_label(state->code, TRUE_LABEL(op_ast->rhs));
        asm_add_instruction(state->code, "mov", "rax", "1");
        asm_add_instruction(state->code, "jmp", LABEL(ast, "_xor"));
        asm_add_label(state->code, FALSE_LABEL(op_ast->rhs));
        asm_add_instruction(state->code, "mov", "rax", "0");
        // xor
        asm_add_label(state->code, LABEL(ast, "_xor"));
        asm_add_instruction(state->code, "pop", "rdx");
        asm_add_instruction(state->code, "xor", "rax", "rdx");
        asm_add_instruction(state->code, "jnz", TRUE_LABEL(ast));
        asm_add_instruction(state->code, "jmp", FALSE_LABEL(ast));
        break;
    case BooleanOperationKind::Not:
        compile_ast(state, op_ast->lhs, scope);
        asm_add_label(state->code, FALSE_LABEL(op_ast->lhs));
        asm_add_instruction(state->code, "jmp", TRUE_LABEL(ast));
        asm_add_label(state->code, TRUE_LABEL(op_ast->lhs));
        asm_add_instruction(state->code, "jmp", FALSE_LABEL(ast));
        break;
        // TODO: be carefull with unsigned in the future
    case BooleanOperationKind::Eql:
        compile_cmp(state, ast, scope, "je");
        break;
    case BooleanOperationKind::Inf:
        compile_cmp(state, ast, scope, "jl");
        break;
    case BooleanOperationKind::Sup:
        compile_cmp(state, ast, scope, "jg");
        break;
    case BooleanOperationKind::Ieq:
        compile_cmp(state, ast, scope, "jle");
        break;
    case BooleanOperationKind::Seq:
        compile_cmp(state, ast, scope, "jge");
        break;
    }
}

// TODO: builtin function should not be compiled but linked !
void compile_builtin_function(CompilerState *state, Ast *ast, Scope *) {
    BuiltinFunction *fun_ast = &ast->data.builtin_function;
    switch (fun_ast->kind) {
    case BuiltinFunctionKind::Shw: {
        // TODO: for now we only support text
        std::string msg_id = asm_create_data_id(state->code, "shw_msg");
        std::string msg = fun_ast->argument->data.value.value.string.ptr;
        std::string msg_len = std::to_string(get_compiled_string_size(msg));

        asm_add_data(state->code, msg_id, ".string", msg);
        // save registers?
        asm_add_instruction(state->code, "mov", "rax", "1");
        asm_add_instruction(state->code, "mov", "rdi", "1");
        asm_add_instruction(state->code, "lea", "rsi", msg_id);
        asm_add_instruction(state->code, "mov", "rdx", msg_len);
        asm_add_instruction(state->code, "syscall");
    } break;
    case BuiltinFunctionKind::Ipt:
        // TODO
        break;
    }
}

void compile_function_call(CompilerState *state, Ast *ast, Scope *scope) {
    // WARN: make sure to put the number of float arguments in `al` before
    // calling C variadic functions
    // TODO: implement a system that avoid pushing arguments on the stack
    // [arg_{N}, arg_{N - 1}, arg_{N - 2}, ret_addr, rbp]
    Type *function_type =
        scope_lookup_symbol(scope, ast->data.function_call.name.ptr)->type;
    auto args = ast->data.function_call.arguments;
    auto args_type = function_type->data.function.arguments_types;
    size_t int_idx = 0, flt_idx = 0;
    size_t stack_size_to_release = 0;
    std::stack<Address> args_addr;
    // TODO: check the rules for structs
    for (size_t i = 0; i < args.len; i++) {
        // compile the argument ast
        compile_ast(state, args[i], scope);
        auto value_addr = state->last_expr_addr;

        if (is_flt(args_type[i])) {
            if (int_idx < ARG_REGISTERS_FLOAT.size()) {
                asm_mov(state, ARG_REGISTERS_FLOAT[flt_idx], asm_addr(value_addr));
            } else {
                args_addr.push(value_addr);
            }
            flt_idx++;
        } else if (is_int(args_type[i])) {
            if (int_idx < ARG_REGISTERS_INTEGER.size()) {
                asm_mov(state, ARG_REGISTERS_INTEGER[int_idx], asm_addr(value_addr));
            } else {
                args_addr.push(value_addr);
            }
            int_idx++;
        } else if (is_str(args_type[i])) {
            if (value_addr.addressing_mode == AddressingMode::ImmediateValue) {
                value_addr = create_tmp_str_value(state, args[i], value_addr);
                stack_size_to_release += 16;
            }
            if (int_idx < ARG_REGISTERS_INTEGER.size()) {
                if (value_addr.addressing_mode == AddressingMode::Based) {
                    asm_add_instruction(state->code, "lea",
                                        ARG_REGISTERS_INTEGER[int_idx],
                                        asm_addr(value_addr));
                } else {
                    asm_mov(state, ARG_REGISTERS_INTEGER[int_idx], asm_addr(value_addr));
                }
            } else {
                args_addr.push(value_addr);
            }
            int_idx++;
        } else {
            throw std::logic_error("error: unsuported type " +
                                   type_to_string(args_type[i]));
        }
    }
    // preentively substract to rsp to maintain the alignment
    stack_size_to_release = 8 * args_addr.size(); // TODO: works because the size of the arguments is 8, but we might compute the size at some point
    if (stack_size_to_release % 16 != 0) {
        assert(stack_size_to_release % 16 == 8);
        asm_add_instruction(state->code, "sub", "rsp", "8");
        stack_size_to_release += 8;
    }
    // push the remaining argument on the stack in the reverse order
    while (!args_addr.empty()) {
        asm_add_instruction(state->code, "push", asm_addr(args_addr.top()));
        args_addr.pop();
    }
    // call the function
    asm_add_instruction(state->code, "xor", "rax", "rax");
    asm_add_instruction(state->code, "call", ast->data.function_call.name.ptr);
    // make sure the compiler know that the result will be store in rax (if
    // the function returns a result).
    if (is_flt(function_type->data.function.return_type)) {
        asm_addr_register(state, "xmm0", function_type->data.function.return_type);
    } else {
        asm_addr_register(state, "rax", function_type->data.function.return_type);
    }
    if (stack_size_to_release > 0) {
        asm_add_instruction(state->code, "add", "rsp", std::to_string(stack_size_to_release));
    }
}

void compile_cnd_stmt(CompilerState *state, Ast *ast, Scope *scope) {
    assert(map_contains(scope->child_scopes, ast));
    CndStmt *stmt = &ast->data.cnd_stmt;
    Scope *cnd_scope = scope->child_scopes[ast];

    compile_ast(state, stmt->condition, cnd_scope);
    asm_add_label(state->code, TRUE_LABEL(stmt->condition));
    compile_block(state, stmt->block, cnd_scope);
    asm_add_instruction(state->code, "jmp", END_LABEL(stmt));
    asm_add_label(state->code, FALSE_LABEL(stmt->condition));
    if (stmt->otw) {
        // TODO: check that nested conditions are compiled correctly
        compile_ast(state, stmt->otw, scope);
    }
    asm_add_label(state->code, END_LABEL(stmt));
}

void compile_for_stmt(CompilerState *state, Ast *ast, Scope *scope) {
    assert(map_contains(scope->child_scopes, ast));
    ForStmt *stmt = &ast->data.for_stmt;
    Ast *cnd = stmt->condition;
    Scope *for_scope = scope->child_scopes[ast];

    compile_ast(state, stmt->init, for_scope);
    asm_add_label(state->code, BEGIN_LABEL(stmt));
    compile_ast(state, stmt->condition, for_scope);
    asm_add_label(state->code, TRUE_LABEL(cnd));
    compile_block(state, stmt->block, for_scope);
    compile_ast(state, stmt->step, for_scope);
    asm_add_instruction(state->code, "jmp", BEGIN_LABEL(stmt));
    asm_add_label(state->code, FALSE_LABEL(cnd));
}

void compile_whl_stmt(CompilerState *state, Ast *ast, Scope *scope) {
    assert(map_contains(scope->child_scopes, ast));
    WhlStmt *stmt = &ast->data.whl_stmt;
    Ast *cnd = stmt->condition;
    Scope *whl_scope = scope->child_scopes[ast];

    asm_add_label(state->code, BEGIN_LABEL(stmt));
    compile_ast(state, stmt->condition, whl_scope);
    asm_add_label(state->code, TRUE_LABEL(cnd));
    compile_block(state, stmt->block, whl_scope);
    asm_add_instruction(state->code, "jmp", BEGIN_LABEL(stmt));
    asm_add_label(state->code, FALSE_LABEL(cnd));
}

void compile_ret_stmt(CompilerState *state, Ast *ast, Scope *scope) {
    RetStmt *ret_ast = &ast->data.ret_stmt;

    compile_ast(state, ret_ast->expression, scope);
    if (ret_ast->expression->kind != AstKind::Value) {
        if (is_flt(scope_lookup_expr_type(scope, ret_ast->expression))) {
            asm_mov(state, "xmm0", asm_addr(state->last_expr_addr));
        } else {
            asm_mov(state, "rax", asm_addr(state->last_expr_addr));
        }
    }
    // TODO: this is not required if the return is at the end of the block
    asm_add_instruction(state->code, "jmp",
                        "epilogue_" + state->curr_function_id);
}

void compile_block(CompilerState *state, Ast *ast, Scope *scope) {
    Block *block_ast = &ast->data.block;
    for (auto instruction : block_ast->asts) {
        compile_ast(state, instruction, scope->child_scopes[ast]);
    }
}

void allocate_arguments(CompilerState *state, Ast *ast, Scope *scope) {
    auto args = ast->data.function.arguments;
    size_t int_idx = 0, flt_idx = 0;
    for (size_t idx = 0; idx < args.len; idx++) {
        auto *var_ast = &args[idx]->data.variable_definition;
        auto type = scope_lookup_symbol(scope, var_ast->name.ptr)->type;
        std::string reg = "";

        if (is_int(type) || is_chr(type)) {
            if (int_idx < ARG_REGISTERS_INTEGER.size()) {
                reg = ARG_REGISTERS_INTEGER[int_idx];
            } else {
                reg = "[rbp+" +
                      std::to_string(
                          8 /* (saved rsp) */ + 8 /* (sizeof of the varible (all 8 for now)) */ +
                          8 /* (again, size of type) */ * (int_idx - ARG_REGISTERS_INTEGER.size() + 1)) +
                      "]";
            }
            int_idx++;
        } else if (is_flt(type)) {
            if (int_idx < ARG_REGISTERS_FLOAT.size()) {
                reg = ARG_REGISTERS_FLOAT[flt_idx];
            } else {
                reg = "[rbp+" +
                      std::to_string(
                          8 /* (saved rsp) */ + 8 /* (sizeof of the varible (all 8 for now)) */ +
                          8 /* (again, size of type) */ * (flt_idx - ARG_REGISTERS_FLOAT.size() + 1)) +
                      "]";
            }
            flt_idx++;
        } else {
            std::cerr << "error: not implemented" << std::endl;
        }
        compile_variable_definition(state, args[idx], scope);
        asm_mov(state, asm_addr(get_address(state, var_ast->name.ptr)), reg);
    }
}

void compile_function_definition(CompilerState *state, Ast *ast, Scope *scope) {
    assert(map_contains(scope->child_scopes, ast));
    Function *fund_def_ast = &ast->data.function;
    Scope *function_scope = scope->child_scopes[ast];
    state->curr_function_id = fund_def_ast->name.ptr;
    state->frame_offset = 8;

    asm_add_label(state->code, fund_def_ast->name.ptr);
    asm_add_instruction(state->code, "push", "rbp");
    asm_add_instruction(state->code, "mov", "rbp", "rsp");

    // TODO: optimize this
    allocate_arguments(state, ast, function_scope);

    // TODO: we want to go down the scope
    compile_block(state, fund_def_ast->body, function_scope);

    asm_add_label(state->code, "epilogue_" + std::string(fund_def_ast->name.ptr));
    asm_add_instruction(state->code, "mov", "rsp", "rbp");
    asm_add_instruction(state->code, "pop", "rbp");
    asm_add_instruction(state->code, "ret");
}

void compile_function_declaration(CompilerState *state, Ast *ast, Scope *) {
    asm_add_instruction(state->code, ".extern", ast->data.function.name.ptr);
}

void compile_ast(CompilerState *state, Ast *ast, Scope *scope) {
    switch (ast->kind) {
    case AstKind::Value:
        compile_value(state, ast, scope);
        break;
    case AstKind::VariableDefinition:
        compile_variable_definition(state, ast, scope);
        break;
    case AstKind::VariableReference:
        compile_variable_reference(state, ast, scope);
        break;
    case AstKind::Assignment:
        compile_assignement(state, ast, scope);
        break;
    case AstKind::IndexExpression:
        compile_index_expression(state, ast, scope);
        break;
    case AstKind::Function:
        if (ast->data.function.body != nullptr) {
            compile_function_definition(state, ast, scope);
        } else {
            compile_function_declaration(state, ast, scope);
        }
        break;
    case AstKind::FunctionCall:
        compile_function_call(state, ast, scope);
        break;
    case AstKind::CndStmt:
        compile_cnd_stmt(state, ast, scope);
        break;
    case AstKind::WhlStmt:
        compile_whl_stmt(state, ast, scope);
        break;
    case AstKind::ForStmt:
        compile_for_stmt(state, ast, scope);
        break;
    case AstKind::RetStmt:
        compile_ret_stmt(state, ast, scope);
        break;
    case AstKind::Block:
        compile_block(state, ast, scope);
        break;
    case AstKind::ArithmeticOperation:
        compile_arithmetic_operation(state, ast, scope);
        break;
    case AstKind::BooleanOperation:
        compile_boolean_operation(state, ast, scope);
        break;
    case AstKind::BuiltinFunction:
        compile_builtin_function(state, ast, scope);
        break;
    }
}

void make_start(CompilerState *state) {
    asm_add_global_symbol(state->code, "_start");
    asm_add_label(state->code, "_start");
    asm_add_instruction(state->code, "xor", "ebp", "ebp");
    // TODO: args
    asm_add_instruction(state->code, "xor", "rax", "rax");
    asm_add_instruction(state->code, "call", "main");
    asm_add_instruction(state->code, "mov", "rdi", "rax");
    asm_add_instruction(state->code, "mov", "rax", "60");
    asm_add_instruction(state->code, "syscall");
}

void compile(CompilerState *state, Program const &program) {
    for (auto ast : program.code) {
        compile_ast(state, ast, program.scope);
    }
    if (scope_lookup_symbol(program.scope, "main")) {
        make_start(state);
    }
}

} // namespace gnu_linux

} // namespace x86_64

} // end namespace compiler
