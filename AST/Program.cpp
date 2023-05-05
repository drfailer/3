#include "Program.hpp"
#include <memory>

Program::Program()
{
  includes = std::list<std::string>();
  functions = std::list<std::shared_ptr<Function>>();
}

void Program::addInclude(std::string i) {
  includes.push_back(i);
}

void Program::addFunction(std::shared_ptr<Function> f) {
  functions.push_back(f);
}

void Program::display()
{
  for (std::string i : includes)
    std::cout << "include " << i << std::endl;
  for (std::shared_ptr<Function> f : functions)
    f->display();
}

std::list<std::string> Program::getIncludes() const
{
  return includes;
}

std::list<std::shared_ptr<Function>> Program::getFunctions() const
{
  return functions;
}

void Program::compile(std::ofstream& fs)
{
  fs << "#!/usr/bin/env python3" << std::endl;
  fs << "# generated using ISIMA's transpiler" << std::endl << std::endl;
  for (std::shared_ptr<Function> function : functions) {
    function->compile(fs, 0);
    fs << std::endl;
  }
  fs << std::endl
     << "if __name__ == '__main__':" << std::endl
     << "\tmain()";
}

