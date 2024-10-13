#ifndef STATEMENTS_H
#define STATEMENTS_H
#include "node.hpp"
#include "typednodes.hpp"
#include <memory>
#include <list>

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
             std::shared_ptr<Block> instructions, std::list<PrimitiveType> type)
        : id_(id), parameters_(parameters), type_(type), block_(instructions) {}
    PrimitiveType type() const override { return type_.back(); }
    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::string id_;
    std::list<Variable> parameters_;
    std::list<PrimitiveType> type_;
    std::shared_ptr<Block> block_ = nullptr;
};

/**
 * @brief  Cnd statement.
 */
class Cnd : public Node {
  public:
    Cnd(std::shared_ptr<Node> condition, std::shared_ptr<Block> block)
        : condition_(condition), block_(block), elseBlock_(nullptr) {}

    void elseBlock(std::shared_ptr<Block> block) { elseBlock_ = block; }

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
class Whl : public Node {
  public:
    Whl(std::shared_ptr<Node> condition, std::shared_ptr<Block> block)
        : condition_(condition), block_(block) {}

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::shared_ptr<Node> condition_ = nullptr;
    std::shared_ptr<Block> block_ = nullptr;
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
