#ifndef FACTORS_H
#define FACTORS_H
#include "node.hpp"
#include "type_system/types.hpp"
#include <memory>

/**
 * @brief  Interface for typed nodes.
 */
struct TypedNode : Node {
    TypedNode(type_system::type type) : type(type) {}
    virtual ~TypedNode() = default;

    type_system::type type = nullptr;
};

/**
 * @brief  Basic values of an available type.
 */
struct Value : TypedNode {
    Value(type_system::LiteralValue value, type_system::type type)
        : TypedNode(type), value(value) {}

    void compile(std::ofstream &, int) override;
    void display() override;

    type_system::LiteralValue value;
};

/**
 * @brief  Reference to a variable.
 */
struct Variable : TypedNode {
    Variable(std::string const &id, type_system::type type)
        : TypedNode(type), id(id) {}

    void display() override;
    void compile(std::ofstream &, int) override;

    std::string id;
};

/**
 * @brief  Arrays.
 *         Note: the language supports only 1 dimentional arrays.
 */
struct Array : Variable {
    Array(std::string name, int size, type_system::type type)
        : Variable(name, type), size(size) {}

    int size;
};

/**
 * @brief  Represents the access to a particular element in an array (save the
 *         index which is a node)
 */
struct ArrayAccess : Array {
    ArrayAccess(std::string name, type_system::type type,
                std::shared_ptr<Node> index)
        : Array(name, -1, type), index(index) {}

    void display() override;
    void compile(std::ofstream &, int) override;

    std::shared_ptr<Node> index = nullptr;
};

/**
 * @brief  Calling a function. This function has a name and some parameters. The
 *         parameters are node as the can have multiple types (binary operation,
 *         variable, values, other funcall, ...)
 */
struct FunctionCall : TypedNode {
    FunctionCall(std::string const &functionName,
                 std::list<std::shared_ptr<TypedNode>> const &parameters,
                 type_system::type type)
        : TypedNode(type), functionName(functionName),
          parameters(parameters) {}

    void display() override;
    void compile(std::ofstream &, int) override;

    std::string functionName;
    std::list<std::shared_ptr<TypedNode>> parameters = {};
};

#endif
