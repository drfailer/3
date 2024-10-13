#ifndef DECLARATIONS_H
#define DECLARATIONS_H
#include "node.hpp"
#include "typednodes.hpp"

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

#endif
