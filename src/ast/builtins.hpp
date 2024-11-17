#ifndef BUILTIN_H
#define BUILTIN_H
#include "typednodes.hpp"
#include <memory>

/******************************************************************************/
/*                                     IO                                     */
/******************************************************************************/

/**
 * @brief  Builtin show function that can be used to display a literal string, a
 *         array of chr (3's strings) or a variable of a primitive type.
 */
struct Shw : Node {
    Shw(std::shared_ptr<Node> content) : str(""), content(content) {}

    Shw(std::string str)
        : str(str), content(std::shared_ptr<Node>(nullptr)) {}

    void compile(std::ofstream &, int) override;
    void display() override;

    std::string str;
    std::shared_ptr<Node> content = nullptr;
};

/**
 * @brief  Reads a primitive type to the standard input and store the result in
 *         the specified variable.
 */
struct Ipt : Node {
    Ipt(std::shared_ptr<TypedNode> variable) : variable(variable) {}

    void display() override;
    void compile(std::ofstream &, int) override;

    std::shared_ptr<TypedNode> variable = nullptr;
};

#endif
