#ifndef OPERATORS_H
#define OPERATORS_H
#include "node.hpp"
#include "typednodes.hpp"
#include <memory>

/******************************************************************************/
/*                                 assignment                                 */
/******************************************************************************/

/**
 * @brief  Represent an assignment operation using he 'set' operator.
 */
struct Assignment : Node {
    Assignment(std::shared_ptr<Variable> variable,
               std::shared_ptr<TypedNode> value)
        : variable(variable), value(value) {}

    void display() override;
    void compile(std::ofstream &, int) override;

    std::shared_ptr<Variable> variable = nullptr;
    std::shared_ptr<TypedNode> value = nullptr;
};

/******************************************************************************/
/*                             binary operations                              */
/******************************************************************************/

struct BinaryOperation : Node {
    BinaryOperation(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : left(left), right(right) {}

    void display() override;

    std::shared_ptr<Node> left = nullptr;
    std::shared_ptr<Node> right = nullptr;
};

/******************************************************************************/
/*                           arithmetic operations                            */
/******************************************************************************/

struct AddOP : BinaryOperation, TypedNode {
    AddOP(std::shared_ptr<TypedNode> left, std::shared_ptr<TypedNode> right)
        : BinaryOperation(left, right),
          TypedNode(selectType(left->type, right->type)) {}

    void display() override; // +
    void compile(std::ofstream &, int) override;
};

struct MnsOP : BinaryOperation, TypedNode {
    MnsOP(std::shared_ptr<TypedNode> left, std::shared_ptr<TypedNode> right)
        : BinaryOperation(left, right),
          TypedNode(selectType(left->type, right->type)) {}

    void display() override; // -
    void compile(std::ofstream &, int) override;
};

struct TmsOP : BinaryOperation, TypedNode {
    TmsOP(std::shared_ptr<TypedNode> left, std::shared_ptr<TypedNode> right)
        : BinaryOperation(left, right),
          TypedNode(selectType(left->type, right->type)) {}

    void display() override; // *
    void compile(std::ofstream &, int) override;
};

struct DivOP : BinaryOperation, TypedNode {
    DivOP(std::shared_ptr<TypedNode> left, std::shared_ptr<TypedNode> right)
        : BinaryOperation(left, right),
          TypedNode(selectType(left->type, right->type)) {}

    void display() override; // /
    void compile(std::ofstream &, int) override;
};

/******************************************************************************/
/*                             boolean operations                             */
/******************************************************************************/

struct EqlOP : BinaryOperation {
    EqlOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

struct SupOP : BinaryOperation {
    SupOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

struct InfOP : BinaryOperation {
    InfOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

struct SeqOP : BinaryOperation {
    SeqOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

struct IeqOP : BinaryOperation {
    IeqOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

struct OrOP : BinaryOperation {
    OrOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left || right
    void compile(std::ofstream &, int) override;
};

struct AndOP : BinaryOperation {
    AndOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left && right
    void compile(std::ofstream &, int) override;
};

struct XorOP : BinaryOperation {
    XorOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left ^ right
    void compile(std::ofstream &, int) override;
};

struct NotOP : Node {
    NotOP(std::shared_ptr<Node> param) : param(param) {}

    void display() override; // !param
    void compile(std::ofstream &, int) override;

    std::shared_ptr<Node> param;
};

#endif
