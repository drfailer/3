#include "checks.hpp"
#include "ast.hpp"
#include "scope.hpp"
#include "type.hpp"
#include "tools/messages.hpp"
#include "tools/array.hpp"

bool check(CheckState *state, Ast *ast, Scope *scope);
bool check_block(CheckState *state, Ast *ast, Scope *scope);
bool check_boolean_operation(CheckState *state, Ast *ast, Scope *scope);

Type *type_specifier_to_type(CheckState *state, Scope *scope, TypeSpecifier const &specifier) {
    Type *type = nullptr;

    switch (specifier.kind) {
    case TypeSpecifierKind::Nil: type = state->global_scope->type_table["nil"].type; break;
    case TypeSpecifierKind::Chr: type = state->global_scope->type_table["chr"].type; break;
    case TypeSpecifierKind::Int: type = state->global_scope->type_table["int"].type; break;
    case TypeSpecifierKind::Flt: type = state->global_scope->type_table["flt"].type; break;
    case TypeSpecifierKind::Str: type = state->global_scope->type_table["str"].type; break;
    case TypeSpecifierKind::Obj: type = scope_lookup_type(scope, specifier.name.ptr)->type; break;
    }

    // TODO: update this when we will support dynamic arrays
    if (specifier.size > 0) {
        return new_type(
            state->type_pool,
            TypeKind::Array,
            .array = {
                .element_type = type,
                .size = specifier.size,
                .dynamic = false,
            },
        );
    }
    return type;
}

bool check_value(CheckState *state, Ast *ast, Scope *scope) {
    Type *type = nullptr;

    switch (ast->data.value.kind) {
    case ValueKind::Character:
        type = new_type(state->type_pool, TypeKind::Primitive, .primitive = PrimitiveType::Chr);
        break;
    case ValueKind::Integer:
        type = new_type(state->type_pool, TypeKind::Primitive, .primitive = PrimitiveType::Int);
        break;
    case ValueKind::Real:
        type = new_type(state->type_pool, TypeKind::Primitive, .primitive = PrimitiveType::Flt);
        break;
    case ValueKind::String:
        type = new_type(state->type_pool, TypeKind::Primitive, .primitive = PrimitiveType::Str);
        break;
    }
    scope->expr_types[ast] = type;
    return true;
}

bool check_variable_definition(CheckState *state, Ast *ast, Scope *scope) {
    assert(ast->kind == AstKind::VariableDefinition);
    std::string variable_name = std::string(ast->data.variable_definition.name.ptr);

    if (map_contains(scope->symbol_table, variable_name)) {
        Symbol *sym = &scope->symbol_table[variable_name];
        MULTIPLE_DEFINITION_ERROR(ast->location, variable_name, sym->location);
        return false;
    }
    Type *type = type_specifier_to_type(state, scope, ast->data.variable_definition.type_specifier);
    scope_add_symbol(scope, variable_name, type, ast->location);
    return true;
}

bool check_variable_reference(CheckState *, Ast *ast, Scope *scope) {
    assert(ast->kind == AstKind::VariableReference);
    std::string variable_name = std::string(ast->data.variable_reference.name.ptr);
    Symbol *sym = scope_lookup_symbol(scope, variable_name);

    if (sym == nullptr) {
        UNDEFINED_SYMBOL_ERROR(ast->location, variable_name);
        return false;
    }
    scope->expr_types[ast] = sym->type;
    return true;
}

bool check_assignment(CheckState *state, Ast *ast, Scope *scope) {
    Ast *target = ast->data.assignment.target;
    Ast *expr = ast->data.assignment.value;

    if (!check(state, expr, scope) || !check(state, target, scope)) {
        return false;
    }

    auto expr_type = scope_lookup_expr_type(scope, expr);
    auto target_type = scope_lookup_expr_type(scope, target);

    if (!is_primitive(target_type)) { // may change
        INVALID_MOV_ERROR(ast->location, target_type);
        return false;
    }
    if (!is_convertible(expr_type, target_type)) {
        BAD_ASSIGNMENT_ERROR(ast->location, expr_type, target_type);
        return false;
    }
    if (!equal(expr_type, target_type)) {
        IMPLICIT_CONVERTION_WARNING(ast->location, expr_type, target_type);
    }
    return true;
}

bool check_index_expr(CheckState *state, Ast *ast, Scope *scope) {
    Ast *variable_ast = ast->data.index_expression.element;
    Ast *index_ast = ast->data.index_expression.index;

    if (!check(state, variable_ast, scope) || !check(state, index_ast, scope)) {
        return false;
    }

    auto variable_type = scope_lookup_expr_type(scope, variable_ast);
    if (variable_type->kind != TypeKind::Array) {
        INDEX_NON_ARRAY_TYPE_ERROR(ast->location, std::string(variable_ast->data.variable_reference.name.ptr));
        return false;
    }

    auto index_type = scope_lookup_expr_type(scope, index_ast);
    if (!is_int(index_type)) {
        INVALID_INDEX_TYPE_ERROR(index_ast->location, index_type);
        return false;
    }
    scope->expr_types[ast] = variable_type->data.array.element_type;
    return true;
}

bool check_function_definition(CheckState *state, Ast *ast, Scope *scope) {
    Scope *function_scope = scope_add_child(scope, ast);

    state->current_function = std::string(ast->data.function.name.ptr);

    for (size_t i = 0; i < ast->data.function.arguments.len; ++i) {
        check_variable_definition(state, ast->data.function.arguments[i], function_scope);
    }
    return check_block(state, ast->data.function.body, function_scope);
}

bool check_function_call(CheckState *state, Ast *ast, Scope *scope) {
    auto function_name = ast->data.function_call.name;
    auto args = ast->data.function_call.arguments;
    auto sym = scope_lookup_symbol(scope, function_name.ptr);

    if (sym == nullptr) {
        UNDEFINED_SYMBOL_ERROR(ast->location, function_name.ptr);
        return false;
    }
    if (sym->type->kind != TypeKind::Function) {
        INVALID_CALL_ERROR(ast->location, function_name.ptr);
        return false;
    }

    auto function_type = &sym->type->data.function;
    scope->expr_types[ast] = function_type->return_type;
    if (function_type->arguments_types.len != args.len) {
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
        auto found_type = scope_lookup_expr_type(scope, args[idx]);
        auto expected_tpe = args_types[idx];

        if (!is_convertible(found_type, expected_tpe)) {
            ARGUMENT_TYPE_ERROR(ast->location, function_name.ptr, idx,
                                found_type, expected_tpe);
            res = false;
        }
    }
    return res;
}

bool check_cnd_stmt(CheckState *state, Ast *ast, Scope *scope) {
    Ast *block = ast->data.cnd_stmt.block;
    Ast *condition = ast->data.cnd_stmt.condition;
    Ast *otw = ast->data.cnd_stmt.otw;
    Scope *cnd_scope = scope_add_child(scope, ast);

    bool condition_ok = check_boolean_operation(state, condition, cnd_scope);
    bool block_ok = check_block(state, block, cnd_scope);
    bool otw_ok = otw != nullptr ? check(state, otw, scope) : true;
    return condition_ok && block_ok && otw_ok;
}

bool check_for_stmt(CheckState *state, Ast *ast, Scope *scope) {
    Location location = ast->location;
    Ast *init = ast->data.for_stmt.init;
    Ast *step = ast->data.for_stmt.step;
    Ast *block = ast->data.for_stmt.block;
    Scope *for_scope = scope_add_child(scope, ast);

    if (!check(state, init, for_scope) || !check(state, step, for_scope)) {
        return false;
    }

    assert(step->kind == AstKind::Assignment);

    Ast *idx_var = init->data.assignment.target;
    Symbol *idx_sym = scope_lookup_symbol(for_scope, idx_var->data.variable_reference.name.ptr);
    Type *idx_var_type = idx_sym->type;
    Type *step_type = scope_lookup_expr_type(for_scope, step->data.assignment.value);

    if (!equal(idx_var_type, step_type)) {
        if (!is_convertible(idx_var_type, step_type)) {
            FOR_STEP_TYPE_ERROR(location, step_type, idx_var_type);
            return false;
        } else {
            FOR_STEP_TYPE_WARNING(location, step_type, idx_var_type);
        }
    }
    return check_block(state, block, for_scope);
}

bool check_whl_stmt(CheckState *state, Ast *ast, Scope *scope) {
    Ast *condition = ast->data.whl_stmt.condition;
    Ast *block = ast->data.whl_stmt.block;
    Scope *whl_scope = scope_add_child(scope, ast);
    bool condition_ok = check_boolean_operation(state, condition, whl_scope);
    bool block_ok = check_block(state, block, whl_scope);
    return condition_ok && block_ok;
}

bool check_ret_stmt(CheckState *state, Ast *ast, Scope *scope) {
    Symbol *sym = scope_lookup_symbol(scope, state->current_function);
    assert(sym && "current function not inserted in the symbol table.");
    auto expected_type = sym->type->data.function.return_type;
    Ast *expr = ast->data.ret_stmt.expression;

    if (is_nil(expected_type) && expr != nullptr) {
        ERROR(ast->location, "value returned in " << QUOTE(state->current_function)
                << " which should return nil.")
    } else if (!is_nil(expected_type) && expr == nullptr) {
        ERROR(ast->location, "invalid nil return in " << QUOTE(state->current_function)
                << ", expected value of type "
                << QUOTE(type_to_string(expected_type)) << ".")
    }

    if (expr == nullptr) {
        return true;
    }

    if (!check(state, expr, scope)) {
        return false;
    }

    auto expr_type = scope_lookup_expr_type(scope, expr);
    if (!is_convertible(expr_type, expected_type)) {
        INVALID_RETURN_TYPE_ERROR(ast->location, state->current_function, expr_type,
                                  sym->type->data.function.return_type);
        return false;
    } else if (!equal(expr_type, expected_type)) {
        IMPLICIT_CONVERTION_WARNING(ast->location, expr_type,
                                    sym->type->data.function.return_type);
    }
    return true;
}

bool check_block(CheckState *state, Ast *ast, Scope *scope) {
    Scope *block_scope = scope_add_child(scope, ast);
    bool ok = true;
    for (Ast *instruction : ast->data.block.asts) {
        if (!check(state, instruction, block_scope)) {
            ok = false;
        }
    }
    return ok;
}

bool check_arithmetic_operation(CheckState *state, Ast *ast, Scope *scope) {
    Ast *lhs = ast->data.arithmetic_operation.lhs;
    Ast *rhs = ast->data.arithmetic_operation.rhs;

    if (!check(state, lhs, scope) || !check(state, rhs, scope)) {
        return false;
    }

    auto lhs_type = scope_lookup_expr_type(scope, lhs);
    auto rhs_type = scope_lookup_expr_type(scope, rhs);

    if (!supports_arithmetic(lhs_type) || !supports_arithmetic(rhs_type)) {
        ARITHMETIC_OPERATOR_ERROR(ast->location, operator_name(ast->data.arithmetic_operation.kind))
        return false;
    }
    scope->expr_types[ast] = select_most_precise_arithmetic_type(lhs_type, rhs_type);
    return true;
}

bool check_boolean_operation(CheckState *state, Ast *ast, Scope *scope) {
    Ast *lhs = ast->data.boolean_operation.lhs;
    Ast *rhs = ast->data.boolean_operation.rhs;

    if (rhs == nullptr) {
        return check(state, lhs, scope);
    }

    if (!check(state, lhs, scope) || !check(state, rhs, scope)) {
        return false;
    }
    auto lhs_type = scope_lookup_expr_type(scope, lhs);
    auto rhs_type = scope_lookup_expr_type(scope, rhs);

    assert(lhs_type != nullptr);
    assert(rhs_type != nullptr);

    if (!equal(lhs_type, rhs_type)) {
        if (is_convertible(lhs_type, rhs_type)) {
            IMPLICIT_CONVERTION_WARNING(ast->location, lhs_type, rhs_type);
        } else {
            return false;
        }
    }
    // TODO: we need a bool type!
    return true;
}

bool check_builtin_function(CheckState *state, Ast *ast, Scope *scope) {
    Ast *arg = ast->data.builtin_function.argument;

    if (!check(state, arg, scope)) {
        return false;
    }

    if (ast->data.builtin_function.kind == BuiltinFunctionKind::Shw) {
        auto expr_type = scope_lookup_expr_type(scope, arg);
        if (!is_str(expr_type)) {
            ERROR(ast->location, "shw takes a string as argument.");
            return false;
        }
        // TODO: this may change if in the future implementation of shw
        // support variable display
    }
    return true;
}

bool check(CheckState *state, Ast *ast, Scope *scope) {
    bool ok = true;

    if (ast == nullptr) {
        std::cerr << "internal error: cannot check a nullptr ast." << std::endl;
        return false;
    }

    switch (ast->kind) {
    case AstKind::Value:
        ok = check_value(state, ast, scope);
        break;
    case AstKind::VariableDefinition:
        ok = check_variable_definition(state, ast, scope);
        break;
    case AstKind::VariableReference:
        ok = check_variable_reference(state, ast, scope);
        break;
    case AstKind::Assignment:
        ok = check_assignment(state, ast, scope);
        break;
    case AstKind::IndexExpression:
        ok = check_index_expr(state, ast, scope);
        break;
    case AstKind::Function:
        if (ast->data.function.body != nullptr) {
            ok = check_function_definition(state, ast, scope);
        }
        break;
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
    case AstKind::Block: {
        check_block(state, ast, scope);
    } break;
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

Type *function_type_from_ast(CheckState *state, Scope *scope, Ast *ast) {
    assert(ast->kind == AstKind::Function);
    Type *return_type = type_specifier_to_type(state, scope, ast->data.function.return_type_specifier);
    size_t nb_args = ast->data.function.arguments.len;
    Array<Type *> arguments_types = array_create<Type *>(nb_args, nb_args, state->allocator);

    for (size_t i = 0; i < ast->data.function.arguments.len; ++i) {
        Ast *arg = ast->data.function.arguments[i];
        assert(arg->kind == AstKind::VariableDefinition);
        arguments_types[i] = type_specifier_to_type(state, scope, arg->data.variable_definition.type_specifier);
    }
    return new_type(
        state->type_pool,
        TypeKind::Function,
        .function = {
            .return_type = return_type,
            .arguments_types = arguments_types,
        }
    );
}

// First pass in which global symbols are added to the symbol table.
bool process_global_symbols(CheckState *state, std::vector<Ast *> const &program, Scope *global_scope) {
    for (Ast *ast : program) {
        assert(ast->kind == AstKind::Function);
        std::string function_name = std::string(ast->data.function.name.ptr);
        Symbol *sym = scope_lookup_symbol(global_scope, function_name);

        if (sym != nullptr) {
            MULTIPLE_DEFINITION_ERROR(ast->location, function_name, sym->location);
            return false;
        }
        Type *function_type = function_type_from_ast(state, global_scope, ast);
        scope_add_symbol(global_scope, function_name, function_type, ast->location);
    }
    return true;
}

// TODO: we should create the symbol table here
bool check(CheckState *state, Program const &program) {
    bool ok = true;

    if (!process_global_symbols(state, program.code, program.scope)) {
        return false;
    }

    for (Ast *ast : program.code) {
        if (!check(state, ast, program.scope)) {
            ok = false;
        }
    }
    return ok;
}
