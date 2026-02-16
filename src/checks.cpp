#include "checks.hpp"
#include "ast.hpp"
#include "symbol_table.hpp"
#include "type/predicates.hpp"
#include "type/type.hpp"
#include "tools/messages.hpp"

bool check(CheckState *state, Ast *ast, SymbolTable *scope);
bool check_block(CheckState *state, Ast *ast, SymbolTable *scope);
bool check_boolean_operation(CheckState *state, Ast *ast, SymbolTable *scope);

bool check_assignment(CheckState *state, Ast *ast, SymbolTable *scope) {
    Ast *target = ast->data.assignment.target;
    Ast *expr = ast->data.assignment.value;

    if (!check(state, expr, scope) || !check(state, target, scope)) {
        return false;
    }

    auto expr_type = lookup_ast_type(scope, expr);
    auto target_type = lookup_ast_type(scope, target);

    if (!type::is_convertible(expr_type, target_type)) {
        BAD_ASSIGNMENT_ERROR(ast->location, expr_type, target_type);
        return false;
    }
    if (!type::equal(expr_type, target_type)) {
        IMPLICIT_CONVERTION_WARNING(ast->location, expr_type, target_type);
    }
    return true;
}

bool check_index_expr(CheckState *state, Ast *ast, SymbolTable *scope) {
    Ast *variable_ast = ast->data.index_expression.element;
    Ast *index_ast = ast->data.index_expression.index;

    if (!check(state, variable_ast, scope) || !check(state, index_ast, scope)) {
        return false;
    }

    auto index_type = lookup_ast_type(scope, index_ast);
    if (!type::is_int(index_type)) {
        INVALID_INDEX_TYPE_ERROR(index_ast->location, index_type);
        return false;
    }
    return true;
}

bool check_function_definition(CheckState *state, Ast *ast, SymbolTable *scope) {
    state->current_function = std::string(ast->data.function_definition.name.ptr);
    SymbolTable *function_scope = scope->symbols.at(state->current_function).scope;
    return check_block(state, ast->data.function_definition.body, function_scope);
}

bool check_function_call(CheckState *state, Ast *ast, SymbolTable *scope) {
    auto function_name = ast->data.function_call.name;
    auto args = ast->data.function_call.arguments;
    auto sym = lookup_id(scope, function_name.ptr);

    if (sym == nullptr) {
        UNDEFINED_SYMBOL_ERROR(ast->location, function_name.ptr);
        return false;
    }
    if (sym->type->kind != type::TypeKind::Function) {
        INVALID_CALL_ERROR(ast->location, function_name.ptr);
        return false;
    }

    auto function_type = sym->type->value.function;
    scope->ast_types.insert({ast, function_type->return_type});
    if (function_type->arguments_types.size() != args.len) {
        WRONG_NUMBER_OF_ARGUMENT_ERROR(ast->location, function_name.ptr,
                                       sym->type);
        return false;
    }

    bool res = true;
    auto args_types = function_type->arguments_types;
    for (size_t idx = 0; idx < args.len; ++idx) {
        if (!check(state, args[idx], scope)) {
            return false;
        }
        auto found_type = lookup_ast_type(scope, args[idx]);
        auto expected_tpe = args_types[idx];

        if (!type::is_convertible(found_type, expected_tpe)) {
            ARGUMENT_TYPE_ERROR(ast->location, function_name.ptr, idx,
                                found_type, expected_tpe);
            res = false;
        }
    }
    return res;
}

bool check_cnd_stmt(CheckState *state, Ast *ast, SymbolTable *scope) {
    Ast *block = ast->data.cnd_stmt.block;
    Ast *condition = ast->data.cnd_stmt.condition;
    Ast *otw = ast->data.cnd_stmt.otw;
    bool condition_ok = check_boolean_operation(state, condition, scope->block_scopes[block]);
    bool block_ok = check_block(state, block, scope->block_scopes[block]);
    bool otw_ok = otw != nullptr ? check(state, otw, scope) : true;
    return condition_ok && block_ok && otw_ok;
}

bool check_for_stmt(CheckState *state, Ast *ast, SymbolTable *scope) {
    Location location = ast->location;
    Ast *init = ast->data.for_stmt.init;
    Ast *step = ast->data.for_stmt.step;

    if (!check(state, init, scope) || !check(state, step, scope)) {
        return false;
    }

    assert(step->kind == AstKind::Assignment);

    Ast *block = ast->data.for_stmt.block;
    Ast *idx_var = init->data.assignment.target;
    Symbol *idx_sym = lookup_id(scope, idx_var->data.variable_reference.name.ptr);
    type::Type *idx_var_type = idx_sym->type;
    type::Type *step_type = lookup_ast_type(scope, step->data.assignment.value);

    if (!type::equal(idx_var_type, step_type)) {
        if (!type::is_convertible(idx_var_type, step_type)) {
            FOR_STEP_TYPE_ERROR(location, step_type, idx_var_type);
            return false;
        } else {
            FOR_STEP_TYPE_WARNING(location, step_type, idx_var_type);
        }
    }
    return check_block(state, block, scope->block_scopes[block]);
}

bool check_whl_stmt(CheckState *state, Ast *ast, SymbolTable *scope) {
    Ast *condition = ast->data.whl_stmt.condition;
    Ast *block = ast->data.whl_stmt.block;
    bool condition_ok = check_boolean_operation(state, condition, scope);
    bool block_ok = check_block(state, block, scope->block_scopes[block]);
    return condition_ok && block_ok;
}

bool check_ret_stmt(CheckState *state, Ast *ast, SymbolTable *scope) {
    Symbol *sym = lookup_id(scope, state->current_function);
    assert(sym && "current function not inserted in the symbol table.");
    auto expected_type = sym->type->value.function->return_type;
    Ast *expr = ast->data.ret_stmt.expression;

    if (expr == nullptr) {
        if (!type::is_nil(expected_type)) {
            INVALID_RETURN_TYPE_ERROR(
                ast->location, sym->id,
                sym->type->value.function->return_type, expected_type);
            return false;
        }
        return true;
    }

    if (!check(state, expr, scope)) {
        return false;
    }

    auto expr_type = lookup_ast_type(scope, expr);
    if (!type::is_convertible(expr_type, expected_type)) {
        INVALID_RETURN_TYPE_ERROR(ast->location, sym->id, expr_type,
                                  sym->type->value.function->return_type);
        return false;
    } else if (!type::equal(expr_type, expected_type)) {
        IMPLICIT_CONVERTION_WARNING(ast->location, expr_type,
                                    sym->type->value.function->return_type);
    }
    return true;
}

bool check_block(CheckState *state, Ast *ast, SymbolTable *scope) {
    bool ok = true;
    for (Ast *instruction : ast->data.block.asts) {
        if (!check(state, instruction, scope)) {
            ok = false;
        }
    }
    return ok;
}

bool check_arithmetic_operation(CheckState *state, Ast *ast, SymbolTable *scope) {
    Ast *lhs = ast->data.arithmetic_operation.lhs;
    Ast *rhs = ast->data.arithmetic_operation.rhs;

    if (!check(state, lhs, scope) || !check(state, rhs, scope)) {
        return false;
    }

    auto lhs_type = lookup_ast_type(scope, lhs);
    auto rhs_type = lookup_ast_type(scope, rhs);

    if (!type::supports_arithmetic(lhs_type) || !type::supports_arithmetic(rhs_type)) {
        ARITHMETIC_OPERATOR_ERROR(ast->location, operator_name(ast->data.arithmetic_operation.kind))
        return false;
    }
    scope->ast_types.insert({
            ast,
            type::select_most_precise_arithmetic_type(lhs_type, rhs_type)
    });
    return true;
}

bool check_boolean_operation(CheckState *state, Ast *ast, SymbolTable *scope) {
    Ast *lhs = ast->data.boolean_operation.lhs;
    Ast *rhs = ast->data.boolean_operation.rhs;

    if (!check(state, lhs, scope) || !check(state, rhs, scope)) {
        return false;
    }
    auto lhs_type = lookup_ast_type(scope, lhs);
    auto rhs_type = lookup_ast_type(scope, rhs);

    assert(lhs_type != nullptr);
    assert(rhs_type != nullptr);

    if (!type::equal(lhs_type, rhs_type)) {
        if (type::is_convertible(lhs_type, rhs_type)) {
            IMPLICIT_CONVERTION_WARNING(ast->location, lhs_type, rhs_type);
        } else {
            return false;
        }
    }
    return true;
}

bool check_builtin_function(CheckState *state, Ast *ast, SymbolTable *scope) {
    Ast *arg = ast->data.builtin_function.argument;

    if (!check(state, arg, scope)) {
        return false;
    }

    if (ast->data.builtin_function.kind == BuiltinFunctionKind::Shw) {
        auto expr_type = lookup_ast_type(scope, arg);
        if (!type::is_str(expr_type)) {
            ERROR(ast->location, "shw takes a string as argument.");
            return false;
        }
        // TODO: this may change if in the future implementation of shw
        // support variable display
    }
    return true;
}

bool check(CheckState *state, Ast *ast, SymbolTable *scope) {
    bool ok = true;

    if (ast == nullptr) {
        std::cerr << "internal error: cannot check a nullptr ast." << std::endl;
        return false;
    }

    switch (ast->kind) {
    case AstKind::Value: break;
    case AstKind::VariableDefinition: break;
    case AstKind::VariableReference: break;
    case AstKind::Assignment:
        ok = check_assignment(state, ast, scope);
        break;
    case AstKind::IndexExpression:
        ok = check_index_expr(state, ast, scope);
        break;
    case AstKind::FunctionDefinition:
        ok = check_function_definition(state, ast, scope);
        break;
    case AstKind::FunctionDeclaration: break;
    case AstKind::FunctionCall:
        ok = check_function_call(state, ast, scope);
        break;
    case AstKind::CndStmt:
        ok = check_cnd_stmt(state, ast, scope);
        break;
    case AstKind::WhlStmt:
        ok = check_whl_stmt(state, ast, scope);
        break;
    case AstKind::ForStmt:
        ok = check_for_stmt(state, ast, scope);
        break;
    case AstKind::RetStmt:
        ok = check_ret_stmt(state, ast, scope);
        break;
    case AstKind::Block:
        ok = check_block(state, ast, scope->block_scopes[ast]);
        break;
    case AstKind::ArithmeticOperation:
        ok = check_arithmetic_operation(state, ast, scope);
        break;
    case AstKind::BooleanOperation:
        ok = check_boolean_operation(state, ast, scope);
        break;
    case AstKind::BuiltinFunction:
        ok = check_builtin_function(state, ast, scope);
        break;
    }
    return ok;
}

bool check(std::vector<Ast *> program, SymbolTable *symtable) {
    bool ok = true;
    CheckState state;

    for (Ast *ast : program) {
        if (!check(&state, ast, symtable)) {
            ok = false;
        }
    }
    return ok;
}
