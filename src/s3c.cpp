#include "s3c.hpp"
#include "symbol_table.hpp"
#include "tools/messages.hpp"
#include "tools/type_utilities.hpp"
#include "ast.hpp"
#include "type/predicates.hpp"
#include "type/type.hpp"
#include <sstream>

// TODO: add an error status in the state

namespace s3c {

State *state_create() {
    auto *state = new State();
    state->symtable = symbol_table_create(nullptr);
    state->status = 0;
    state->arena = arena_create();
    state->allocator = arena_allocator(&state->arena);
    mem_pool_init(&state->ast_pool, 100);
    return state;
}

void state_destroy(State *state) {
    // BUG: does not work, the memory must get corrupted somewhere
    symbol_table_destroy(state->symtable);
    mem_pool_destroy(&state->ast_pool);
    arena_destroy(&state->arena);
    delete state;
}

void enter_file(State *state, std::string const &filename) {
    if (filename.empty()) {
        return;
    }

    state->curr_filename = filename[0] == '"' ? filename.substr(1) : filename;
    if (filename.back() == '"') {
        state->curr_filename.pop_back();
    }
}

void add_function(State *state, Ast *ast) {
    state->program.push_back(ast);
}

Ast *new_arithmetic_operation(State *state, Ast *lhs, Ast *rhs,
                              ArithmeticOperationKind kind, size_t line) {
    auto op_ast = new_ast(
        &state->ast_pool,
        LOCATION,
        AstKind::ArithmeticOperation,
        .arithmetic_operation = ArithmeticOperation{kind, lhs, rhs}
    );
    return op_ast;
}

Ast *new_variable_reference(State *state, std::string const &name,
                            size_t line) {
    return new_ast(
        &state->ast_pool,
        LOCATION,
        AstKind::VariableReference,
        .variable_reference = VariableReference{
            .name = string_create(name, state->allocator),
        }
    );
}

Ast *new_index_expr(State *state, std::string const &name, size_t line,
                    Ast *index_ast) {
    auto location = location_create(state->curr_filename, line);
    auto variable = new_ast(
        &state->ast_pool,
        location,
        AstKind::VariableReference,
        .variable_reference = VariableReference{
            .name = string_create(name, state->allocator),
        }
    );
    return new_ast(
        &state->ast_pool,
        location,
        AstKind::IndexExpression,
        .index_expression = IndexExpression{
            .element = variable,
            .index = index_ast,
        }
    );
}

Ast *new_for(State *state, Ast *init, Ast *end,
                    Ast *step, Ast *block, size_t line) {
    auto location = LOCATION;
    // the syntax allow to just put the expression that is assigned to the loop
    // variable, therefore, we need to manually create the assignment
    auto step_assignment = new_ast(
        &state->ast_pool,
        location,
        AstKind::Assignment,
        .assignment = Assignment{ init->data.assignment.target, step }
    );
    return new_ast(&state->ast_pool, location, AstKind::ForStmt,
                    .for_stmt = ForStmt{
                        .init = init,
                        .condition = end,
                        .step = step_assignment,
                        .block = block,
                    });
}

bool try_verify_main_type(State *state) {
    auto sym = lookup_id(state->symtable, "main");

    if (!sym) {
        return true; // when compiling libraries or object files
    }

    if (sym->type->kind != type::TypeKind::Function) {
        ERROR(sym->location, "main should be a function.")
        return false;
    }

    if (!type::is_int(sym->type->value.function->return_type)) {
        ERROR(sym->location, "invalid return type for main.")
        return false;
    }

    return true;
}

} // end namespace s3c
