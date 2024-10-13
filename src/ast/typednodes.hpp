#ifndef FACTORS_H
#define FACTORS_H
#include "node.hpp"
#include "typesystem/types.hpp"
#include <memory>

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

/**
 * @brief  Calling a function. This function has a name and some parameters. The
 *         parameters are node as the can have multiple types (binary operation,
 *         variable, values, other funcall, ...)
 */
class FunctionCall : public TypedNode {
  public:
    FunctionCall(std::string const &functionName,
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

#endif
