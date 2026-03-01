#include "s3c.hpp"
#include "scope.hpp"
#include "tools/messages.hpp"
#include "ast.hpp"
#include "type.hpp"
#include "type.hpp"
#include <sstream>

// create type table entries for the builtin types
// Note: doing this avoid allocating multiple pointers to the same primitive
// type. It is also more coherent
void init_global_scope(State *state) {
    scope_add_type(state->global_scope, "nil", new_type(&state->type_pool, TypeKind::Nil, {}));
    scope_add_type(state->global_scope, "chr", new_type(&state->type_pool, TypeKind::Primitive, .primitive = PrimitiveType::Chr));
    scope_add_type(state->global_scope, "int", new_type(&state->type_pool, TypeKind::Primitive, .primitive = PrimitiveType::Int));
    scope_add_type(state->global_scope, "flt", new_type(&state->type_pool, TypeKind::Primitive, .primitive = PrimitiveType::Flt));
    scope_add_type(state->global_scope, "str", new_type(&state->type_pool, TypeKind::Primitive, .primitive = PrimitiveType::Str));
}

State *state_create() {
    auto *state = new State();
    state->global_scope = scope_create();
    state->status = 0;
    state->arena = arena_create();
    state->allocator = arena_allocator(&state->arena);
    // TODO: use the default pool size (should be a define)
    mem_pool_init(&state->ast_pool, 100);
    mem_pool_init(&state->type_pool, 100);
    init_global_scope(state);
    return state;
}

void state_destroy(State *state) {
    scope_destroy(state->global_scope);
    mem_pool_destroy(&state->ast_pool);
    mem_pool_destroy(&state->type_pool);
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

Ast *new_boolean_operation(State *state, Ast *lhs, Ast *rhs,
                           BooleanOperationKind kind, size_t line) {
    return new_ast(
        &state->ast_pool,
        location_create(state->curr_filename, line),
        AstKind::BooleanOperation,
        .boolean_operation = {
            .kind = kind,
            .lhs = lhs,
            .rhs = rhs,
        }
    );
}

bool try_verify_main_type(State *state) {
    auto sym = scope_lookup_symbol(state->global_scope, "main");

    if (!sym) {
        return true; // when compiling libraries or object files
    }

    if (sym->type->kind != TypeKind::Function) {
        ERROR(sym->location, "main should be a function.")
        return false;
    }

    if (!is_int(sym->type->data.function.return_type)) {
        ERROR(sym->location, "invalid return type for main.")
        return false;
    }

    return true;
}
