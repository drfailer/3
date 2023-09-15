#ifndef __PROGRAM__
#define __PROGRAM__
#include "AST.hpp"
#include <algorithm>
#include <fstream>

/******************************************************************************/
/*                                  program                                   */
/******************************************************************************/

class Program {
      public:
        std::list<std::shared_ptr<Function>> getFunctions() const;
        std::list<std::string> getIncludes() const;
        bool isIncluded(std::string lib) const {
                return std::find(includes.begin(), includes.end(), lib) !=
                       includes.end();
        }
        Program();
        void addInclude(std::string);
        void addFunction(std::shared_ptr<Function>);
        void compile(std::ofstream &);
        void display();

      private:
        std::list<std::string> includes;
        std::list<std::shared_ptr<Function>> functions;
};

#endif
