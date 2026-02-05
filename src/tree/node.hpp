#ifndef AST_NODE_H
#define AST_NODE_H
#include "location.hpp"
#include "../tools/array.hpp"
#include "../tools/string.hpp"
#include "../tools/mem.hpp"
#include <cstddef>
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

enum class ValueKind {
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
        String string;
    } value;
};

struct VariableDefinition {
    String name;
};

struct VariableReference {
    String name;
};

struct Assignment {
    Node *target;
    Node *value;
};

struct IndexExpression {
    Node *element;
    Node *index;
};

struct FunctionDefinition {
    String name;
    Array<Node *> arguments = {};
    Node *body;
};

// TODO: do we want to use this type in FunctionDefinition as well?
struct FunctionDeclaration {
    String name;
    Array<Node *> arguments = {};
};

struct FunctionCall {
    String name;
    Array<Node *> arguments = {};
};

struct CndStmt {
    Node *condition;
    Node *block;
    Node *otw;
};

struct WhlStmt {
    Node *condition;
    Node *block;
};

struct ForStmt {
    Node *init;
    Node *condition;
    Node *step;
    Node *block;
};

struct RetStmt {
    Node *expression;
};

struct Block {
    Array<Node *> nodes = {};
};

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

enum BuiltinFunctionKind {
    Shw,
    Ipt,
};
struct BuiltinFunction {
    BuiltinFunctionKind kind;
    Node *argument;
};

union NodeData {
    Value value;
    VariableDefinition variable_definition;
    VariableReference variable_reference;
    Assignment assignment;
    IndexExpression index_expression;
    FunctionDefinition function_definition;
    FunctionDeclaration function_declaration;
    FunctionCall function_call;
    CndStmt cnd_stmt;
    WhlStmt whl_stmt;
    ForStmt for_stmt;
    RetStmt ret_stmt;
    Block block;
    ArithmeticOperation arithmetic_operation;
    BooleanOperation boolean_operation;
    BuiltinFunction builtin_function;
};

struct Node {
    Location location;
    NodeKind kind;
    NodeData data;
};

void print_node(Node *node);

inline Node *new_node_(MemPool<Node> *pool, Location loc, NodeKind kind, NodeData data) {
    return mem_pool_alloc(pool, node::Node{
        .location = loc,
        .kind = kind,
        .data = data,
    });
}

} // end namespace node

#define new_node(pool, loc, kind, ...) node::new_node_((pool), (loc), (kind), node::NodeData{__VA_ARGS__})

#endif
