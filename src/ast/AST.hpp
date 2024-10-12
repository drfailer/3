#ifndef __AST__
#define __AST__
#include "Types.hpp"
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
    virtual ~Node() = 0;
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
    virtual ~TypedNode() = 0;
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
    Variable(std::string id, Type type) : TypedNode(type), id_(id) { }
    std::string const &id() const { return id_; }

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::string id_;
};

class Array : public Variable {
  public:
    std::string getId() const;
    int getSize() const;
    Array(std::string, int, Type);
    ~Array() = default;

  protected:
    // TODO: this should be unsigned
    int size;
};

class ArrayDeclaration : public Array {
  public:
    void display() override;
    void compile(std::ofstream &, int) override;
    ArrayDeclaration(std::string, int, Type);
    ~ArrayDeclaration() = default;
};

class ArrayAccess : public Array {
  public:
    void display() override;
    void compile(std::ofstream &, int) override;
    std::shared_ptr<Node> getIndex() const;
    ArrayAccess(std::string, Type, std::shared_ptr<Node>);
    ~ArrayAccess() = default;

  private:
    std::shared_ptr<Node> index;
};

/******************************************************************************/
/*                                 commands                                   */
/******************************************************************************/

/**
 * @brief  Represent an assignment operation. The "ifdef" check of the
 *         variable is done in the parser using the table of symbols.
 */
class Assignment : public Node {
  public:
    std::shared_ptr<Variable> getVariable() const { return variable; }
    std::shared_ptr<TypedNode> getValue() const { return value; }
    void display() override;
    void compile(std::ofstream &, int) override;
    Assignment(std::shared_ptr<Variable>, std::shared_ptr<TypedNode>);

  private:
    std::shared_ptr<Variable> variable;
    std::shared_ptr<TypedNode> value;
};

/**
 * @brief  Declare a variable.
 *
 * NOTE: could be nice to have a type here.
 */
class Declaration : public Node {
  public:
    void display() override;
    void compile(std::ofstream &, int) override;
    Declaration(Variable);

  private:
    Variable variable;
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
    std::string getFunctionName() const;
    void compile(std::ofstream &, int) override;
    std::list<std::shared_ptr<TypedNode>> getParams() const;
    Funcall(std::string, std::list<std::shared_ptr<TypedNode>>, Type);
    void display() override;

  private:
    std::string functionName;
    std::list<std::shared_ptr<TypedNode>> params;
};

// TODO: change this name
/******************************************************************************/
/*                                 statements                                 */
/******************************************************************************/

/**
 * @brief  A `Statement` is somthing that contains a `Block` (function
 *         definitions, if, for, while).
 */
class Statement : public Node {
  public:
    Statement(std::shared_ptr<Block>);
    void display() override;

  protected:
    std::shared_ptr<Block> block;
};

/**
 * @brief  Fonction declaration. It has an id (function name), some parameters
 *         (which are just variable declarations) and a return type.
 *
 * TODO: create a return statment and manage return type.
 */
class Function : public Statement, public TypedNode {
  public:
    void display() override;
    void compile(std::ofstream &, int) override;
    Type type() const override;
    Function(std::string, std::list<Variable>, std::shared_ptr<Block>,
             std::list<Type>);

  private:
    std::string id;
    std::list<Variable> params;
    std::list<Type> type_;
};

/**
 * @brief  If declaration. It has a condition which is a boolean operation (ref:
 *         parser).
 */
class If : public Statement {
  public:
    If(std::shared_ptr<Node>, std::shared_ptr<Block>);
    void createElse(std::shared_ptr<Block>);
    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::shared_ptr<Node> condition; // TODO: put real conditions
    std::shared_ptr<Block> elseBlock;
};

/**
 * @brief  For declaration. The for loop has a "range" which has a begin, an end
 *         and a step value. It also has a loop variable.
 */
class For : public Statement {
  public:
    For(Variable, std::shared_ptr<Node>, std::shared_ptr<Node>,
        std::shared_ptr<Node>, std::shared_ptr<Block>);
    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    Variable var;
    std::shared_ptr<Node> begin;
    std::shared_ptr<Node> end;
    std::shared_ptr<Node> step;
};

/**
 * @brief  While declaration. The while loop just has a condition.
 */
class While : public Statement {
  public:
    While(std::shared_ptr<Node>, std::shared_ptr<Block>);
    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::shared_ptr<Node> condition; // TODO: create condition class
};

/******************************************************************************/
/*                           arithmetic operations                            */
/******************************************************************************/

class BinaryOperation : public Node {
  public:
    BinaryOperation(std::shared_ptr<Node>, std::shared_ptr<Node>);
    void display() override;

  protected:
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;
};

class AddOP : public BinaryOperation, public TypedNode {
  public:
    AddOP(std::shared_ptr<TypedNode>, std::shared_ptr<TypedNode>);
    void display() override; // +
    void compile(std::ofstream &, int) override;
};

class MnsOP : public BinaryOperation, public TypedNode {
  public:
    MnsOP(std::shared_ptr<TypedNode>, std::shared_ptr<TypedNode>);
    void display() override; // -
    void compile(std::ofstream &, int) override;
};

class TmsOP : public BinaryOperation, public TypedNode {
  public:
    TmsOP(std::shared_ptr<TypedNode>, std::shared_ptr<TypedNode>);
    void display() override; // *
    void compile(std::ofstream &, int) override;
};

class DivOP : public BinaryOperation, public TypedNode {
  public:
    DivOP(std::shared_ptr<TypedNode>, std::shared_ptr<TypedNode>);
    void display() override; // /
    void compile(std::ofstream &, int) override;
};

/******************************************************************************/
/*                             boolean operations                             */
/******************************************************************************/

class EqlOP : public BinaryOperation {
  public:
    EqlOP(std::shared_ptr<Node>, std::shared_ptr<Node>);
    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

class SupOP : public BinaryOperation {
  public:
    SupOP(std::shared_ptr<Node>, std::shared_ptr<Node>);
    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

class InfOP : public BinaryOperation {
  public:
    InfOP(std::shared_ptr<Node>, std::shared_ptr<Node>);
    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

class SeqOP : public BinaryOperation {
  public:
    SeqOP(std::shared_ptr<Node>, std::shared_ptr<Node>);
    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

class IeqOP : public BinaryOperation {
  public:
    IeqOP(std::shared_ptr<Node>, std::shared_ptr<Node>);
    void display() override; // left == right
    void compile(std::ofstream &, int) override;
};

class OrOP : public BinaryOperation {
  public:
    OrOP(std::shared_ptr<Node>, std::shared_ptr<Node>);
    void display() override; // left || right
    void compile(std::ofstream &, int) override;
};

class AndOP : public BinaryOperation {
  public:
    AndOP(std::shared_ptr<Node>, std::shared_ptr<Node>);
    void display() override; // left && right
    void compile(std::ofstream &, int) override;
};

class XorOP : public BinaryOperation {
  public:
    XorOP(std::shared_ptr<Node>, std::shared_ptr<Node>);
    void display() override; // left ^ right
    void compile(std::ofstream &, int) override;
};

class NotOP : public Node {
  public:
    NotOP(std::shared_ptr<Node>);
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
    void display() override;
    Print(std::string);
    Print(std::shared_ptr<Node>);
    void compile(std::ofstream &, int) override;

  private:
    std::string str;
    std::shared_ptr<Node> content;
};

/**
 * @brief  Gestion de la saisie clavier.
 */
class Read : public Node {
  public:
    void display() override;
    Read(std::shared_ptr<TypedNode>);
    void compile(std::ofstream &, int) override;

  private:
    std::shared_ptr<TypedNode> variable;
};

/******************************************************************************/
/*                                   return                                   */
/******************************************************************************/

class Return : public Node {
  public:
    void display() override;
    void compile(std::ofstream &, int) override;
    Return(std::shared_ptr<Node>);

  private:
    std::shared_ptr<Node> returnExpr;
};

#endif
