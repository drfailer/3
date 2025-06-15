#ifndef AST_NODE_H
#define AST_NODE_H
#include "location.hpp"
#include <cstddef>
#include <list>
#include <string>
#include <vector>

namespace node {

struct Node;
struct Block;

enum class NodeKind {
    Value,
    // variable
    VariableDefinition,
    VariableReference,
    Assignment,
    IndexExpression,
    // funtions
    FunctionDefinition,
    FunctionDeclaration,
    FunctionCall,
    // control flow
    CndStmt,
    WhlStmt,
    ForStmt,
    RetStmt,
    Block,
    // Operations
    ArithmeticOperation,
    BooleanOperation,
    // builtins
    BuiltinFunction,
};

enum ValueKind {
    Character,
    Integer,
    Real,
    String,
};
struct Value {
    ValueKind kind;
    union {
        char character;
        long integer;
        double real;
        const char *string;
    } value;
};
Node *create_value(Location location, char character);
Node *create_value(Location location, long integer);
Node *create_value(Location location, double real);
Node *create_value(Location location, std::string const string);

struct VariableDefinition {
    std::string name;
};
Node *create_variable_definition(Location location, std::string const &name);

struct VariableReference {
    std::string name;
};
Node *create_variable_reference(Location location, std::string const &name);

struct Assignment {
    Node *target;
    Node *value;
};
Node *create_assignment(Location location, Node *target, Node *value);

struct IndexExpression {
    Node *element;
    Node *index;
};
Node *create_index_expression(Location location, Node *element, Node *index);

struct FunctionDefinition {
    std::string name;
    std::list<Node *> arguments;
    Block *body;
};
Node *create_function_definition(Location location, std::string const &name,
                                 std::list<Node *> const &arguments,
                                 Block *body);

struct FunctionCall {
    std::string name;
    std::list<Node *> arguments;
};
Node *create_function_call(Location location, std::string const &name,
                           std::list<Node *> const &arguments);

struct CndStmt {
    Node *condition;
    Block *block;
    Node *else_expression;
};
Node *create_cnd_stmt(Location location, Node *condition, Block *block,
                      Node *else_expression = nullptr);

struct WhlStmt {
    Node *condition;
    Block *block;
};
Node *create_whl_stmt(Location location, Node *condition, Block *block);

struct ForStmt {
    Node *index;
    Node *start;
    Node *end;
    Node *step;
    Block *block;
};
Node *create_for_stmt(Location location, Node *index, Node *start, Node *end,
                      Node *step, Block *block);

struct RetStmt {
    Node *expression;
};
Node *create_ret_stmt(Location location, Node *expression);

struct Block {
    std::vector<Node *> nodes;
};
Node *create_block(Location location, std::vector<Node *> &&nodes);
Node *create_block(Location location, Block *block);

enum ArithmeticOperationKind {
    Add,
    Sub,
    Mul,
    Div,
};
struct ArithmeticOperation {
    ArithmeticOperationKind kind;
    Node *lhs;
    Node *rhs;
};
Node *create_arithmetic_operation(Location location,
                                  ArithmeticOperationKind kind, Node *lhs,
                                  Node *rhs);

enum BooleanOperationKind {
    // arithmetic
    Lor,
    Xor,
    And,
    Not,
    // comparison
    Eql,
    Sup,
    Inf,
    Seq,
    Ieq,
};
struct BooleanOperation {
    BooleanOperationKind kind;
    Node *lhs;
    Node *rhs;
};
Node *create_boolean_operation(Location location, BooleanOperationKind kind,
                               Node *lhs, Node *rhs);

enum BuiltinFunctionKind {
    Shw,
    Ipt,
};
struct BuiltinFunction {
    BuiltinFunctionKind kind;
    Node *argument;
};
Node *create_builtin_function(Location location, BuiltinFunctionKind kind,
                              Node *argument);

struct Node {
    Location location;
    NodeKind kind;
    union {
        Value *value;
        VariableDefinition *variable_definition;
        VariableReference *variable_reference;
        Assignment *assignment;
        IndexExpression *index_expression;
        FunctionDefinition *function_definition;
        FunctionCall *function_call;
        CndStmt *cnd_stmt;
        WhlStmt *whl_stmt;
        ForStmt *for_stmt;
        RetStmt *ret_stmt;
        Block *block;
        ArithmeticOperation *arithmetic_operation;
        BooleanOperation *boolean_operation;
        BuiltinFunction *builtin_function;
    } value;
};

void delete_node(Location location, Node *node);
void print_node(Node *node);

} // end namespace node

#endif
