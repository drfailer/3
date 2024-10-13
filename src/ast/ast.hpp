#ifndef AST_H
#define AST_H
#include "typesystem/types.hpp"
#include <cstdio>
#include <fstream>
#include <list>
#include <memory>
#include <string>

/******************************************************************************/
/*                                    node                                    */
/******************************************************************************/

/**
 * @brief  Generic AST node class.
 */
class Node {
  public:
    virtual ~Node() = default;
    virtual void compile(std::ofstream &, int) = 0;
    virtual void display() = 0;
};

/******************************************************************************/
/*                                   block                                    */
/******************************************************************************/

/**
 * @brief  Define a block of instructions.
 */
class Block : public Node {
  public:
    Block() = default;

    std::shared_ptr<Node> lastNode() { return instructions_.back(); }
    void add(std::shared_ptr<Node> instruction) {
        instructions_.push_back(instruction);
    }

    void compile(std::ofstream &, int = 0) override;
    void display() override;

  private:
    std::list<std::shared_ptr<Node>> instructions_ = {};
};

/******************************************************************************/
/*                                  factors                                   */
/******************************************************************************/

/**
 * @brief  Interface for typed nodes.
 */
class TypedNode : public Node {
  public:
    TypedNode(Type type = VOID) : type_(type) {}
    virtual ~TypedNode() = default;

    virtual Type type() const { return type_; }
    void type(Type type) { this->type_ = type; }

  protected:
    Type type_ = VOID;
};

/**
 * @brief  Basic values of an available type.
 */
class Value : public TypedNode {
  public:
    Value(LiteralValue value, Type type) : TypedNode(type), value_(value) {}
    Value() : TypedNode() {}
    LiteralValue const &value() const { return value_; }

    void compile(std::ofstream &, int) override;
    void display() override;

  private:
    LiteralValue value_;
};

/**
 * @brief  Reference to a variable.
 */
class Variable : public TypedNode {
  public:
    Variable(std::string id, Type type) : TypedNode(type), id_(id) {}
    std::string const &id() const { return id_; }

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::string id_;
};

/**
 * @brief  Arrays.
 *         Note: the language supports only 1 dimentional arrays.
 */
class Array : public Variable {
  public:
    Array(std::string name, int size, Type type)
        : Variable(name, type), size_(size) {}
    int size() const { return size_; }

  protected:
    int size_;
};

/**
 * @brief  Represents array declaration. This class is just used to print the
 *         AST but not for the compilation.
 */
class ArrayDeclaration : public Array {
  public:
    ArrayDeclaration(std::string name, int size, Type type)
        : Array(name, size, type) {}

    void display() override;
    void compile(std::ofstream &, int) override;
};

/**
 * @brief  Represents the access to a particular element in an array (save the
 *         index which is a node)
 */
class ArrayAccess : public Array {
  public:
    ArrayAccess(std::string name, Type type, std::shared_ptr<Node> index)
        : Array(name, -1, type), index_(index) {}
    std::shared_ptr<Node> index() const { return index_; }

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::shared_ptr<Node> index_ = nullptr;
};

/******************************************************************************/
/*                                 commands                                   */
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

/**
 * @brief  Variable declaration.
 */
class Declaration : public Node {
  public:
    Declaration(Variable variable) : variable_(variable) {}

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    Variable variable_;
};

/******************************************************************************/
/*                                  funcall                                   */
/******************************************************************************/

/**
 * @brief  Calling a function. This function has a name and some parameters. The
 *         parameters are node as the can have multiple types (binary operation,
 *         variable, values, other funcall, ...)
 */
class Funcall : public TypedNode {
  public:
    Funcall(std::string const &functionName,
            std::list<std::shared_ptr<TypedNode>> const &params, Type type)
        : TypedNode(type), functionName_(functionName), params_(params) {}

    std::list<std::shared_ptr<TypedNode>> const &params() const {
        return params_;
    }
    std::string const &functionName() const { return functionName_; }

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::string functionName_;
    std::list<std::shared_ptr<TypedNode>> params_ = {};
};

// TODO: change this name
/******************************************************************************/
/*                                 statements                                 */
/******************************************************************************/

/**
 * @brief  Fonction declaration. It has an id (function name), some parameters
 *         (which are just variable declarations) and a return type.
 *
 * TODO: create a return statment and manage return type.
 */
class Function : public TypedNode {
  public:
    Function(std::string id, std::list<Variable> parameters,
             std::shared_ptr<Block> instructions, std::list<Type> type)
        : id_(id), parameters_(parameters), type_(type), block_(instructions) {}
    Type type() const override { return type_.back(); }
    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::string id_;
    std::list<Variable> parameters_;
    std::list<Type> type_;
    std::shared_ptr<Block> block_ = nullptr;
};

/**
 * @brief  Cnd statement.
 */
class If : public Node {
  public:
    If(std::shared_ptr<Node> condition, std::shared_ptr<Block> block)
        : condition_(condition), block_(block), elseBlock_(nullptr) {}

    void createElse(std::shared_ptr<Block> block) { elseBlock_ = block; }

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::shared_ptr<Node> condition_ = nullptr;
    std::shared_ptr<Block> block_ = nullptr;
    std::shared_ptr<Block> elseBlock_ = nullptr;
};

/**
 * @brief  For statement. The for loop contains the loop variable, and the
 *         expressions that determine the begining, the end and the step of the
 *         loop.
 */
class For : public Node {
  public:
    For(Variable variable, std::shared_ptr<Node> begin,
        std::shared_ptr<Node> end, std::shared_ptr<Node> step,
        std::shared_ptr<Block> block)
        : variable_(variable), begin_(begin), end_(end), step_(step),
          block_(block) {}

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    Variable variable_;
    std::shared_ptr<Node> begin_;
    std::shared_ptr<Node> end_;
    std::shared_ptr<Node> step_;
    std::shared_ptr<Block> block_ = nullptr;
};

/**
 * @brief  While declaration. The while loop just has a condition.
 */
class While : public Node {
  public:
    While(std::shared_ptr<Node> condition, std::shared_ptr<Block> block)
        : condition_(condition), block_(block) {}

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::shared_ptr<Node> condition_ = nullptr;
    std::shared_ptr<Block> block_ = nullptr;
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

/******************************************************************************/
/*                                     IO                                     */
/******************************************************************************/

/**
 * @brief  Gestion de l'affichage. On ne peut afficher que soit une chaine de
 *         caratères (`"str"`), soit une variable.
 */
class Print : public Node {
  public:
    Print(std::shared_ptr<Node> content) : str_(""), content_(content) {}

    Print(std::string str)
        : str_(str), content_(std::shared_ptr<Node>(nullptr)) {}

    void compile(std::ofstream &, int) override;
    void display() override;

  private:
    std::string str_;
    std::shared_ptr<Node> content_ = nullptr;
};

/**
 * @brief  Gestion de la saisie clavier.
 */
class Read : public Node {
  public:
    Read(std::shared_ptr<TypedNode> variable) : variable_(variable) {}

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::shared_ptr<TypedNode> variable_ = nullptr;
};

/******************************************************************************/
/*                                   return                                   */
/******************************************************************************/

class Return : public Node {
  public:
    Return(std::shared_ptr<Node> returnExpr) : returnExpr_(returnExpr) {}

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::shared_ptr<Node> returnExpr_ = nullptr;
};

#endif
