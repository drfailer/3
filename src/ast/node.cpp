#include "node.hpp"

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

Node *create_variable_definition(Location location, std::string const &name) {
    return new_node(VariableDefinition, variable_definition, name);
}

Node *create_variable_reference(Location location, std::string const &name) {
    return new_node(VariableReference, variable_reference, name);
}

Node *create_assignment(Location location, std::string const &name,
                        Node *value) {
    return new_node(Assignment, assignment, name, value);
}

Node *create_index_expression(Location location, std::string const &id,
                              Node *index) {
    return new_node(IndexExpression, index_expression, id, index);
}

Node *create_function_definition(Location location, std::string &name,
                                 std::list<std::string> &&arguments,
                                 Block *block) {
    return new_node(FunctionDefinition, function_definition, name, arguments,
                    block);
}

Node *create_function_call(Location location, std::string const &name,
                           std::list<Node *> &&arguments) {
    return new_node(FunctionCall, function_call, name, arguments);
}

Node *create_cnd_stmt(Location location, Node *condition, Block *block) {
    return new_node(CndStmt, cnd_stmt, condition, block);
}

Node *create_whl_stmt(Location location, Node *condition, Block *block) {
    return new_node(WhlStmt, whl_stmt, condition, block);
}

Node *create_for_stmt(Location location, std::string const &id, Node *start,
                      Node *end, Node *step, Block *block) {
    return new_node(ForStmt, for_stmt, id, start, end, step, block);
}

Node *create_ret_stmt(Location location, Node *expression) {
    return new_node(RetStmt, ret_stmt, expression);
}

Node *create_block(Location location, std::vector<Node *> &&nodes) {
    return new_node(Block, block, nodes);
}

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

} // end namespace node

#undef create_node
