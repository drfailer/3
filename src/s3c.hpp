#ifndef TOOLS_S3C_H
#define TOOLS_S3C_H
#include "symbol_table.hpp"
#include "ast.hpp"
#include "type/type.hpp"
#include "tools/mem.hpp"
#include <cstring>
#include <functional>
#include <stack>

#define LOCATION location_create(state->curr_filename, line)

namespace s3c {

struct State {
    int status;
    std::string curr_filename;
    struct {
        std::string name;
        std::vector<Ast *> arguments;
        type::Type *return_type;
        std::vector<type::Type *> arguments_types;
        std::stack<Ast *> blocks;
    } curr_function;
    struct {
        SymbolTable *curr;
        SymbolTable *global;
    } scopes;
    std::vector<Ast *> program;
    std::vector<std::vector<Ast *>> funcall_parameters;
    std::vector<std::string> funcall_ids;
    std::stack<Ast *> parser_stack; // ast stack for complexe rules
    MemPool<Ast> ast_pool; // TODO: the ast should be default constructible!
    Arena arena;
    Allocator allocator;
};

State *state_create();
void state_destroy(State *state);

void reset_curr_function(State *state);

void enter_file(State *state, std::string const &filename);
void enter_function(State *state, std::string const &function_name);

void enter_scope(State *state);
void leave_scope(State *state, Ast *block);
void add_symbol(State *state, std::string const &id, type::Type *type,
                Location const &location);
void add_global_symbol(State *state, std::string const &id, type::Type *type,
                       SymbolTable *scope, Location const &location);

Ast *new_argument_declaration(State *state, std::string const &id,
                                     type::Type *type, size_t line);
void new_function_definition(State *state, std::string const &id, size_t line);
void set_curr_function_type(State *state, type::Type *return_type, size_t line);
void add_function_definition(State *state, std::string const &name,
                             Ast *body, size_t line);
void add_function_declaration(State *state, size_t line);

void begin_block(State *state);
Ast *end_block(State *state);
void add_instruction(State *state, Ast *ast);

void new_return_expr(State *state, Ast *expr, size_t line);

Ast *new_arithmetic_operation(State *state, Ast *lhs, Ast *rhs,
                              ArithmeticOperationKind kind, size_t line,
                              std::string const &operatorName);

void begin_new_funcall(State *state);
void save_function_call_argument(State *state, Ast *ast);
Ast *new_function_call(State *state, std::string const &function_name,
                              size_t line);

void new_variable_declaration(State *state, std::string id, type::Type *type,
                              size_t line);
Ast *new_variable_reference(State *state, std::string const &name,
                                   size_t line);
Ast *new_index_expr(State *state, std::string const &name, size_t line,
                           Ast *index_ast);

void very_assignement_type(State *state, Assignment *assignment,
                           FunctionCall *expr, std::string const &file,
                           size_t line);

Ast *new_assignment(State *state, Ast *target, Ast *expr,
                    size_t line);

void new_cnd(State *state, Ast *cond, size_t line);
void new_otw(State *state);
void new_otw_cnd(State *state, Ast *cond, size_t line);
Ast * end_cnd(State *state);

Ast *new_for(State *state, Ast *init, Ast *condition,
             Ast *step, Ast *block, size_t line);

void new_shw(State *state, Ast *expr, size_t line);

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
        state->scopes.curr->ast_types.insert(
            {ast, type::create_primitive_type(type::PrimitiveType::Int)});
    } else if constexpr (std::is_same_v<T, double>) {
        ast = new_ast(&state->ast_pool, location, AstKind::Value,
                        .value = Value{
                            .kind = ValueKind::Real,
                            .value = { .real = value },
                        });
        state->scopes.curr->ast_types.insert(
            {ast, type::create_primitive_type(type::PrimitiveType::Flt)});
    } else if constexpr (std::is_same_v<T, char>) {
        ast = new_ast(&state->ast_pool, location, AstKind::Value,
                        .value = Value{
                            .kind = ValueKind::Character,
                            .value = { .character = value },
                        });
        state->scopes.curr->ast_types.insert(
            {ast, type::create_primitive_type(type::PrimitiveType::Chr)});
    } else if constexpr (std::is_same_v<T, std::string>) {
        ast = new_ast(&state->ast_pool, location, AstKind::Value,
                        .value = Value{
                            .kind = ValueKind::String,
                            .value = { .string = string_create(value, state->allocator) },
                        });
        state->scopes.curr->ast_types.insert(
            {ast, type::create_primitive_type(type::PrimitiveType::Str)});
    } else {
        ast = new_ast(&state->ast_pool, location, AstKind::Value,
                        .value = Value{
                            .kind = ValueKind::Integer,
                            .value = { .integer = 0 },
                        });
        state->scopes.curr->ast_types.insert({ast, type::create_nil_type()});
    }
    return ast;
}

} // end namespace s3c

#endif
