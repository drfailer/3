#ifndef TOOLS_S3C_H
#define TOOLS_S3C_H
#include "symbol_table.hpp"
#include "tree/node.hpp"
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
        std::vector<node::Node *> arguments;
        type::Type *return_type;
        std::vector<type::Type *> arguments_types;
        std::stack<node::Node *> blocks;
    } curr_function;
    struct {
        SymbolTable *curr;
        SymbolTable *global;
    } scopes;
    std::vector<node::Node *> program;
    std::vector<std::function<bool(void)>> post_process_callbacks;
    std::vector<std::vector<node::Node *>> funcall_parameters;
    std::vector<std::string> funcall_ids;
    std::stack<node::Node *> parser_stack; // node stack for complexe rules
    MemPool<node::Node> node_pool; // TODO: the node should be default constructible!
    Arena arena;
    Allocator allocator;
};

State *state_create();
void state_destroy(State *state);

bool post_process(State *state);

void reset_curr_function(State *state);

void enter_file(State *state, std::string const &filename);
void enter_function(State *state, std::string const &function_name);

void enter_scope(State *state);
void leave_scope(State *state, node::Node *block);
void add_symbol(State *state, std::string const &id, type::Type *type,
                Location const &location);
void add_global_symbol(State *state, std::string const &id, type::Type *type,
                       SymbolTable *scope, Location const &location);

node::Node *new_argument_declaration(State *state, std::string const &id,
                                     type::Type *type, size_t line);
void new_function_definition(State *state, std::string const &id, size_t line);
void set_curr_function_type(State *state, type::Type *return_type, size_t line);
void add_function_definition(State *state, std::string const &name,
                             node::Node *body, size_t line);
void add_function_declaration(State *state, size_t line);

void begin_block(State *state);
node::Node *end_block(State *state);
void add_instruction(State *state, node::Node *node);

void new_return_expr(State *state, node::Node *expr, size_t line);

node::Node *new_arithmetic_operation(State *state, node::Node *lhs,
                                     node::Node *rhs,
                                     node::ArithmeticOperationKind kind,
                                     size_t line,
                                     std::string const &operatorName);

void begin_new_funcall(State *state);
void save_function_call_argument(State *state, node::Node *node);
node::Node *new_function_call(State *state, std::string const &function_name,
                              size_t line);

void new_variable_declaration(State *state, std::string id, type::Type *type,
                              size_t line);
node::Node *new_variable_reference(State *state, std::string const &name,
                                   size_t line);
node::Node *new_index_expr(State *state, std::string const &name, size_t line,
                           node::Node *index_node);

void very_assignement_type(State *state, node::Assignment *assignment,
                           node::FunctionCall *expr, std::string const &file,
                           size_t line);

node::Node *new_assignment(State *state, node::Node *target, node::Node *expr,
                           size_t line);

void new_cnd(State *state, node::Node *cond, size_t line);
void new_otw(State *state);
void new_otw_cnd(State *state, node::Node *cond, size_t line);
node::Node * end_cnd(State *state);

node::Node *new_for(State *state, node::Node *init, node::Node *condition,
                    node::Node *step, node::Node *block, size_t line);

void new_shw(State *state, node::Node *expr, size_t line);

bool try_verify_main_type(State *state);

template <typename T>
node::Node *new_value(State *state, T value, size_t line) {
    node::Node *node = nullptr;
    auto location = location_create(state->curr_filename, line);

    if constexpr (std::is_same_v<T, long>) {
        node = new_node(&state->node_pool, location, node::NodeKind::Value,
                        .value = node::Value{
                            .kind = node::ValueKind::Integer,
                            .value = { .integer = value },
                        });
        state->scopes.curr->node_types.insert(
            {node, type::create_primitive_type(type::PrimitiveType::Int)});
    } else if constexpr (std::is_same_v<T, double>) {
        node = new_node(&state->node_pool, location, node::NodeKind::Value,
                        .value = node::Value{
                            .kind = node::ValueKind::Integer,
                            .value = { .real = value },
                        });
        state->scopes.curr->node_types.insert(
            {node, type::create_primitive_type(type::PrimitiveType::Flt)});
    } else if constexpr (std::is_same_v<T, char>) {
        node = new_node(&state->node_pool, location, node::NodeKind::Value,
                        .value = node::Value{
                            .kind = node::ValueKind::Integer,
                            .value = { .character = value },
                        });
        state->scopes.curr->node_types.insert(
            {node, type::create_primitive_type(type::PrimitiveType::Chr)});
    } else if constexpr (std::is_same_v<T, std::string>) {
        node = new_node(&state->node_pool, location, node::NodeKind::Value,
                        .value = node::Value{
                            .kind = node::ValueKind::Integer,
                            .value = { .string = string_create(value, state->allocator) },
                        });
        state->scopes.curr->node_types.insert(
            {node, type::create_primitive_type(type::PrimitiveType::Str)});
    } else {
        node = new_node(&state->node_pool, location, node::NodeKind::Value,
                        .value = node::Value{
                            .kind = node::ValueKind::Integer,
                            .value = { .integer = 0 },
                        });
        state->scopes.curr->node_types.insert({node, type::create_nil_type()});
    }
    return node;
}

} // end namespace s3c

#endif
