#ifndef __PROGRAM__
#define __PROGRAM__
#include "AST.hpp"
#include <fstream>

/******************************************************************************/
/*                                  program                                   */
/******************************************************************************/

class Program {
  public:
    Program() = default;
    std::list<std::shared_ptr<Function>> const &functions() const;
    void addFunction(std::shared_ptr<Function>);
    void compile(std::ofstream &);
    void display();

  private:
    std::list<std::shared_ptr<Function>> functions_ = {};
};

#endif
