#ifndef AST_H
#define AST_H
#include "tools/array.hpp"
#include "tools/string.hpp"
#include "tools/mem.hpp"
#include <cstddef>
#include <string>
#include <vector>
#include <ostream>

struct Location {
    size_t row;
    size_t col;
    std::string filename;
};

Location location_create(std::string const &filename, size_t row,
                         size_t col = 0);
std::ostream &operator<<(std::ostream &os, Location const &location);

struct Ast;
struct Block;

enum class AstKind {
    Value,
    // variable
    VariableDefinition,
    VariableReference,
    Assignment,
    IndexExpression,
    // funtions
    Function,
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
    Ast *target;
    Ast *value;
};

struct IndexExpression {
    Ast *element;
    Ast *index;
};

struct Function {
    String name;
    Array<Ast *> arguments = {};
    Ast *body;
};

struct FunctionCall {
    String name;
    Array<Ast *> arguments = {};
};

struct CndStmt {
    Ast *condition;
    Ast *block;
    Ast *otw;
};

struct WhlStmt {
    Ast *condition;
    Ast *block;
};

struct ForStmt {
    Ast *init;
    Ast *condition;
    Ast *step;
    Ast *block;
};

struct RetStmt {
    Ast *expression;
};

struct Block {
    Array<Ast *> asts = {};
};

enum ArithmeticOperationKind {
    Add,
    Sub,
    Mul,
    Div,
};
struct ArithmeticOperation {
    ArithmeticOperationKind kind;
    Ast *lhs;
    Ast *rhs;
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
    Ast *lhs;
    Ast *rhs;
};

enum BuiltinFunctionKind {
    Shw,
    Ipt,
};
struct BuiltinFunction {
    BuiltinFunctionKind kind;
    Ast *argument;
};

union AstData {
    Value value;
    VariableDefinition variable_definition;
    VariableReference variable_reference;
    Assignment assignment;
    IndexExpression index_expression;
    Function function;
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

struct Ast {
    Location location;
    AstKind kind;
    AstData data;
};

std::string operator_name(ArithmeticOperationKind kind);
void print_ast(Ast *ast);

inline Ast *new_ast_(MemPool<Ast> *pool, Location loc, AstKind kind, AstData data) {
    return mem_pool_alloc(pool, Ast{
        .location = loc,
        .kind = kind,
        .data = data,
    });
}
#define new_ast(pool, loc, kind, ...) new_ast_((pool), (loc), (kind), AstData{__VA_ARGS__})

#endif
