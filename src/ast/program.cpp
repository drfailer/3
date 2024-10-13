#include "ast/program.hpp"
#include <memory>
#include <iostream>

void Program::addFunction(std::shared_ptr<Function> f) {
    functions_.push_back(f);
}

void Program::display() {
    for (std::shared_ptr<Function> f : functions_) {
        f->display();
    }
}

std::list<std::shared_ptr<Function>> const &Program::functions() const {
    return functions_;
}

void Program::compile(std::ofstream &fs) {
    fs << "#!/usr/bin/env python3" << std::endl;
    fs << "# generated using ISIMA's transpiler" << std::endl << std::endl;
    for (std::shared_ptr<Function> function : functions_) {
        function->compile(fs, 0);
        fs << std::endl;
    }
    fs << std::endl << "if __name__ == '__main__':" << std::endl << "\tmain()";
}
