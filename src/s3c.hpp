#ifndef TOOLS_S3C_H
#define TOOLS_S3C_H
#include "symbol_table.hpp"
#include "tree/node.hpp"
#include "type/type.hpp"
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
        std::list<node::Node *> arguments;
        type::Type *return_type;
        std::list<type::Type *> arguments_types;
        std::stack<node::Block *> blocks;
    } curr_function;
    struct {
        SymbolTable *curr;
        SymbolTable *global;
    } scopes;
    std::list<node::Node *> program;
    std::list<std::function<bool(void)>> post_process_callbacks;
    std::list<std::list<node::Node *>> funcall_parameters;
    std::list<std::string> funcall_ids;
};

State *state_create();

bool post_process(State *state);

void reset_curr_function(State *state);

void enter_file(State *state, std::string const &filename);
void enter_function(State *state, std::string const &function_name);

void enter_scope(State *state);
void leave_scope(State *state, node::Block *block);
void add_symbol(State *state, std::string const &id, type::Type *type,
                Location const &location);
void add_global_symbol(State *state, std::string const &id, type::Type *type,
                       SymbolTable *scope, Location const &location);

node::Node *new_argument_declaration(State *state, std::string const &id,
                                     type::Type *type, size_t line);
void new_function_definition(State *state, std::string const &id, size_t line);
void set_curr_function_type(State *state, type::Type *return_type, size_t line);
void add_function_definition(State *state, std::string const &name,
                             node::Block *body, size_t line);
void add_function_declaration(State *state, size_t line);

void begin_block(State *state);
node::Block *end_block(State *state);
void add_instruction(State *state, node::Node *node);

void new_return_expr(State *state, node::Node *expr, size_t line);

node::Node *new_arithmetic_operation(State *state, node::Node *lhs,
                                     node::Node *rhs,
                                     node::ArithmeticOperationKind kind,
                                     size_t line,
                                     std::string const &operatorName);

void very_function_call_argument_types(std::string const &functionName,
                                       node::FunctionCall *functionCall,
                                       SymbolTable *scope,
                                       std::string const &fileName,
                                       size_t line);
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

void new_assignment(State *state, node::Node *target, node::Node *expr,
                    size_t line);

node::Node *new_for(State *state, std::string const &index_id,
                    node::Node *begin, node::Node *end, node::Node *step,
                    node::Block *block, size_t line);

void new_shw(State *state, node::Node *expr, size_t line);

bool try_verify_main_type(State *state);

template <typename T>
node::Node *new_value(State *state, T value, size_t line) {
    auto node =
        node::create_value(location_create(state->curr_filename, line), value);

    if (std::is_same_v<T, long>) {
        state->scopes.curr->node_types.insert(
            {node, type::create_primitive_type(type::PrimitiveType::Int)});
    } else if (std::is_same_v<T, double>) {
        state->scopes.curr->node_types.insert(
            {node, type::create_primitive_type(type::PrimitiveType::Flt)});
    } else if (std::is_same_v<T, char>) {
        state->scopes.curr->node_types.insert(
            {node, type::create_primitive_type(type::PrimitiveType::Chr)});
    } else if (std::is_same_v<T, std::string>) {
        state->scopes.curr->node_types.insert(
            {node, type::create_primitive_type(type::PrimitiveType::Str)});
    } else {
        state->scopes.curr->node_types.insert({node, type::create_nil_type()});
    }
    return node;
}

} // end namespace s3c

#endif
