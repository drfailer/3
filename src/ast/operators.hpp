#ifndef OPERATORS_H
#define OPERATORS_H
#include "node.hpp"
#include <memory>

/******************************************************************************/
/*                                 assignment                                 */
/******************************************************************************/

/**
 * @brief  Represent an assignment operation using he 'set' operator.
 */
class Assignment : public Node {
  public:
    Assignment(std::shared_ptr<Variable> variable,
               std::shared_ptr<TypedNode> value)
        : variable_(variable), value_(value) {}

    std::shared_ptr<Variable> variable() const { return variable_; }
    std::shared_ptr<TypedNode> value() const { return value_; }

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::shared_ptr<Variable> variable_ = nullptr;
    std::shared_ptr<TypedNode> value_ = nullptr;
};

/******************************************************************************/
/*                           arithmetic operations                            */
/******************************************************************************/

class BinaryOperation : public Node {
  public:
    BinaryOperation(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : left_(left), right_(right) {}

    void display() override;

  protected:
    std::shared_ptr<Node> left_ = nullptr;
    std::shared_ptr<Node> right_ = nullptr;
};

class AddOP : public BinaryOperation, public TypedNode {
  public:
    AddOP(std::shared_ptr<TypedNode> left, std::shared_ptr<TypedNode> right)
        : BinaryOperation(left, right),
          TypedNode(selectType(left->type(), right->type())) {}

    void display() override; // +
    void compile(std::ofstream &, int) override;
};

class MnsOP : public BinaryOperation, public TypedNode {
  public:
    MnsOP(std::shared_ptr<TypedNode> left, std::shared_ptr<TypedNode> right)
        : BinaryOperation(left, right),
          TypedNode(selectType(left->type(), right->type())) {}

    void display() override; // -
    void compile(std::ofstream &, int) override;
};

class TmsOP : public BinaryOperation, public TypedNode {
  public:
    TmsOP(std::shared_ptr<TypedNode> left, std::shared_ptr<TypedNode> right)
        : BinaryOperation(left, right),
          TypedNode(selectType(left->type(), right->type())) {}

    void display() override; // *
    void compile(std::ofstream &, int) override;
};

class DivOP : public BinaryOperation, public TypedNode {
  public:
    DivOP(std::shared_ptr<TypedNode> left, std::shared_ptr<TypedNode> right)
        : BinaryOperation(left, right),
          TypedNode(selectType(left->type(), right->type())) {}

    void display() override; // /
    void compile(std::ofstream &, int) override;
};

/******************************************************************************/
/*                             boolean operations                             */
/******************************************************************************/

class EqlOP : public BinaryOperation {
  public:
    EqlOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

class SupOP : public BinaryOperation {
  public:
    SupOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

class InfOP : public BinaryOperation {
  public:
    InfOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

class SeqOP : public BinaryOperation {
  public:
    SeqOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

class IeqOP : public BinaryOperation {
  public:
    IeqOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

class OrOP : public BinaryOperation {
  public:
    OrOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left || right
    void compile(std::ofstream &, int) override;
};

class AndOP : public BinaryOperation {
  public:
    AndOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left && right
    void compile(std::ofstream &, int) override;
};

class XorOP : public BinaryOperation {
  public:
    XorOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : BinaryOperation(left, right) {}

    void display() override; // left ^ right
    void compile(std::ofstream &, int) override;
};

class NotOP : public Node {
  public:
    NotOP(std::shared_ptr<Node> param) : param(param) {}

    void display() override; // !param
    void compile(std::ofstream &, int) override;

  private:
    std::shared_ptr<Node> param;
};

#endif
