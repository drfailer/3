#ifndef NODE_H
#define NODE_H
#include <fstream>

/**
 * @brief  Generic AST node class.
 */
class Node {
  public:
    virtual ~Node() = default;
    // TODO: remove
    virtual void compile(std::ofstream &, int) = 0;
    virtual void display() = 0;
};


#endif
