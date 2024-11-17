#ifndef STATEMENTS_H
#define STATEMENTS_H
#include "node.hpp"
#include "typednodes.hpp"
#include <list>
#include <memory>

/******************************************************************************/
/*                                   block                                    */
/******************************************************************************/

/**
 * @brief  Define a block of instructions.
 */
struct Block : Node {
    Block() = default;

    std::shared_ptr<Node> lastNode() { return instructions.back(); }
    void add(std::shared_ptr<Node> instruction) {
        instructions.push_back(instruction);
    }

    void compile(std::ofstream &, int = 0) override;
    void display() override;

    std::list<std::shared_ptr<Node>> instructions = {};
};

/******************************************************************************/
/*                                 statements                                 */
/******************************************************************************/

/**
 * @brief  Fonction declaration. It has an id (function name), some parameters
 *         (which are just variable declarations) and a return type.
 *
 * TODO: create a return statment and manage return type.
 */
struct Function : TypedNode {
    Function(std::string id, std::list<Variable> parameters,
             std::shared_ptr<Block> instructions, type_system::type type)
        : TypedNode(type), id(id), parameters(parameters),
          block(instructions) {}
    void display() override;
    void compile(std::ofstream &, int) override;

    // todo: override getType and return a static_casted Function type (so we
    // can an a return type)

    std::string id;
    std::list<Variable> parameters;
    std::shared_ptr<Block> block = nullptr;
};

/**
 * @brief  Cnd statement.
 */
struct Cnd : Node {
    Cnd(std::shared_ptr<Node> condition, std::shared_ptr<Block> block)
        : condition(condition), block(block) {}

    void display() override;
    void compile(std::ofstream &, int) override;

    std::shared_ptr<Node> condition = nullptr;
    std::shared_ptr<Block> block = nullptr;
    std::shared_ptr<Block> elseBlock = nullptr;
};

/**
 * @brief  For statement. The for loop contains the loop variable, and the
 *         expressions that determine the begining, the end and the step of the
 *         loop.
 */
struct For : Node {
    For(Variable variable, std::shared_ptr<Node> begin,
        std::shared_ptr<Node> end, std::shared_ptr<Node> step,
        std::shared_ptr<Block> block)
        : variable(variable), begin(begin), end(end), step(step), block(block) {
    }

    void display() override;
    void compile(std::ofstream &, int) override;

    Variable variable;
    std::shared_ptr<Node> begin;
    std::shared_ptr<Node> end;
    std::shared_ptr<Node> step;
    std::shared_ptr<Block> block = nullptr;
};

/**
 * @brief  While declaration. The while loop just has a condition.
 */
struct Whl : Node {
    Whl(std::shared_ptr<Node> condition, std::shared_ptr<Block> block)
        : condition(condition), block(block) {}

    void display() override;
    void compile(std::ofstream &, int) override;

    std::shared_ptr<Node> condition = nullptr;
    std::shared_ptr<Block> block = nullptr;
};

/******************************************************************************/
/*                                   return                                   */
/******************************************************************************/

struct Return : Node {
    Return(std::shared_ptr<Node> returnExpr) : returnExpr(returnExpr) {}

    void display() override;
    void compile(std::ofstream &, int) override;

    std::shared_ptr<Node> returnExpr = nullptr;
};

#endif
