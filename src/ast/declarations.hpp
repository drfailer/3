#ifndef DECLARATIONS_H
#define DECLARATIONS_H
#include "node.hpp"
#include "typednodes.hpp"

/**
 * @brief  Variable declaration.
 */
struct Declaration : Node {
    Declaration(Variable variable) : variable(variable) {}

    void display() override;
    void compile(std::ofstream &, int) override;

    Variable variable;
};

/**
 * @brief  Array declaration.
 *
 * TODO: constructor for dynamic arrays
 */
struct ArrayDeclaration : Array {
    ArrayDeclaration(std::string name, int size, type_system::type_t type)
        : Array(name, size, type) {}

    void display() override;
    void compile(std::ofstream &, int) override;
};

#endif
