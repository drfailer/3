#ifndef TOOLS_S3C_H
#define TOOLS_S3C_H
#include "symbol_table.hpp"
#include "ast.hpp"
#include "type.hpp"
#include "tools/mem.hpp"
#include <cstring>
#include <functional>
#include <stack>

#define LOCATION location_create(state->curr_filename, line)

namespace s3c {

struct State {
    int status;
    std::string curr_filename;
    SymbolTable *symtable;
    std::vector<Ast *> program;
    MemPool<Ast> ast_pool;
    MemPool<Type> type_pool;
    Arena arena;
    Allocator allocator;
};

State *state_create();
void state_destroy(State *state);

void enter_file(State *state, std::string const &filename);

void add_function(State *state, Ast *function);

Ast *new_arithmetic_operation(State *state, Ast *lhs, Ast *rhs, ArithmeticOperationKind kind, size_t line);
Ast *new_boolean_operation(State *state, Ast *lhs, Ast *rhs, BooleanOperationKind kind, size_t line);

bool try_verify_main_type(State *state);

template <typename T>
Ast *new_value(State *state, T value, size_t line) {
    Ast *ast = nullptr;
    auto location = location_create(state->curr_filename, line);

    if constexpr (std::is_same_v<T, long>) {
        ast = new_ast(&state->ast_pool, location, AstKind::Value,
                        .value = Value{
                            .kind = ValueKind::Integer,
                            .value = { .integer = value },
                        });
    } else if constexpr (std::is_same_v<T, double>) {
        ast = new_ast(&state->ast_pool, location, AstKind::Value,
                        .value = Value{
                            .kind = ValueKind::Real,
                            .value = { .real = value },
                        });
    } else if constexpr (std::is_same_v<T, char>) {
        ast = new_ast(&state->ast_pool, location, AstKind::Value,
                        .value = Value{
                            .kind = ValueKind::Character,
                            .value = { .character = value },
                        });
    } else if constexpr (std::is_same_v<T, std::string>) {
        ast = new_ast(&state->ast_pool, location, AstKind::Value,
                        .value = Value{
                            .kind = ValueKind::String,
                            .value = { .string = string_create(value, state->allocator) },
                        });
    } else {
        ast = new_ast(&state->ast_pool, location, AstKind::Value,
                        .value = Value{
                            .kind = ValueKind::Integer,
                            .value = { .integer = 0 },
                        });
    }
    return ast;
}

} // end namespace s3c

#endif
