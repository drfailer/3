#ifndef TOOLS_S3C_H
#define TOOLS_S3C_H
#include "tree/node.hpp"
#include "symbol_table.hpp"
#include "type/type.hpp"
#include "tools/type_utilities.hpp"
#include <cstring>
#include <functional>
#include <iostream>
#include <sstream>
#include <stack>

#define LOCATION create_location(state->curr_filename, line)

namespace s3c {

struct State {
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
    std::list<std::function<void(void)>> post_process_callbacks;
    std::list<std::list<node::Node *>> funcall_parameters;
    std::list<std::string> funcall_ids;
};

State *state_create();

void post_process(State *state);

void reset_curr_function(State *state);

void enter_file(State *state, std::string const &filename);
void enter_function(State *state, std::string const &function_name);

void enter_scope(State *state);
void leave_scope(State *state);
void add_symbol(State *state, std::string const &id, type::Type *type);
void add_global_symbol(State *state, std::string const &id, type::Type *type);

node::Node *new_argument_declaration(State *state, std::string const id,
                                     type::Type *type, size_t line);
int new_function_definition(State *state, std::string const &id, size_t line);
void set_curr_function_type(State *state, type::Type *return_type);
void add_function_definition(State *state, std::string const &name,
                             node::Block *body, size_t line);

void begin_block(State *state);
node::Block *end_block(State *state);
void add_instruction(State *state, node::Node *node);

void new_return_expr(State *state, node::Node *expr, size_t line);

node::Node *new_arithmetic_operation(State *state, node::Node *lhs,
                                     node::Node *rhs,
                                     node::ArithmeticOperationKind kind,
                                     size_t line,
                                     std::string const &operatorName);

void very_function_call_type(std::string const &functionName,
                             node::FunctionCall *functionCall,
                             SymbolTable *scope, std::string const &fileName,
                             size_t line);
void begin_new_funcall(State *state, std::string const &id);
void save_function_call_argument(State *state, node::Node *node);
// TODO: this function should take the argument list as argument
node::Node *new_function_call(State *state, std::string const &id, size_t line);

void new_variable_declaration(State *state, std::string id, type::Type *type,
                              size_t line);
node::Node *new_variable(State *state, std::string const &name, size_t line);
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

} // end namespace s3c

#endif
