#include "node.hpp"
#include <cstring>
#include <iostream>

#define new_node(type, value_name, ...)                                        \
    new Node{                                                                  \
        .location = location,                                                  \
        .kind = NodeKind::type,                                                \
        .value =                                                               \
            {                                                                  \
                .value_name = new type{__VA_ARGS__},                           \
            },                                                                 \
    };

namespace node {

Node *create_value(Location location, char character) {
    return new_node(Value, value, ValueKind::Character,
                    {.character = character});
}

Node *create_value(Location location, long integer) {
    return new_node(Value, value, ValueKind::Integer, {.integer = integer});
}

Node *create_value(Location location, double real) {
    return new_node(Value, value, ValueKind::Real, {.real = real});
}

Node *create_value(Location location, std::string const string) {
    auto cstring = strdup(string.c_str());
    return new_node(Value, value, ValueKind::String, {.string = cstring});
}

Node *create_variable_definition(Location location, std::string const &name) {
    return new_node(VariableDefinition, variable_definition, name);
}

Node *create_variable_reference(Location location, std::string const &name) {
    return new_node(VariableReference, variable_reference, name);
}

Node *create_assignment(Location location, Node *target, Node *value) {
    return new_node(Assignment, assignment, target, value);
}

Node *create_index_expression(Location location, Node *element, Node *index) {
    return new_node(IndexExpression, index_expression, element, index);
}

Node *create_function_definition(Location location, std::string const &name,
                                 std::vector<Node *> const &arguments,
                                 Node *body) {
    return new_node(FunctionDefinition, function_definition, name, arguments,
                    body);
}

Node *create_function_declaration(Location location, std::string const &name,
                                  std::vector<Node *> const &arguments) {
    return new_node(FunctionDeclaration, function_declaration, name, arguments);
}

Node *create_function_call(Location location, std::string const &name,
                           std::vector<Node *> const &arguments) {
    return new_node(FunctionCall, function_call, name, arguments);
}

Node *create_cnd_stmt(Location location, Node *condition, Node *block,
                      Node *otw) {
    return new_node(CndStmt, cnd_stmt, condition, block, otw);
}

Node *create_whl_stmt(Location location, Node *condition, Node *block) {
    return new_node(WhlStmt, whl_stmt, condition, block);
}

Node *create_for_stmt(Location location, Node *init, Node *condition,
                      Node *step, Node *block) {
    return new_node(ForStmt, for_stmt, init, condition, step, block);
}

Node *create_ret_stmt(Location location, Node *expression) {
    return new_node(RetStmt, ret_stmt, expression);
}

Node *create_block(Location location, std::vector<Node *> &&nodes) {
    return new_node(Block, block, nodes);
}

// Node *create_block(Location location, Node *block) {
//     return new Node{
//         .location = location,
//         .kind = NodeKind::Block,
//         .value = {.block = block},
//     };
// }

Node *create_arithmetic_operation(Location location,
                                  ArithmeticOperationKind kind, Node *lhs,
                                  Node *rhs) {
    return new_node(ArithmeticOperation, arithmetic_operation, kind, lhs, rhs);
}

Node *create_boolean_operation(Location location, BooleanOperationKind kind,
                               Node *lhs, Node *rhs) {
    return new_node(BooleanOperation, boolean_operation, kind, lhs, rhs);
}

Node *create_builtin_function(Location location, BuiltinFunctionKind kind,
                              Node *argument) {
    return new_node(BuiltinFunction, builtin_function, kind, argument);
}

void delete_node(Location location, Node *node) {
    std::cerr << "error: delete_node not implemented" << std::endl;
}

void print_node(Node *node) {
    std::cerr << "error: print_node not implemented" << std::endl;
}

} // end namespace node

#undef create_node
