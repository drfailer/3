#include "ErrorManager.hpp"
#include <ostream>

void ErrorManager::newError(std::string msg) {
        errors = true;
        errStream << "\033[1;31merror\033[0m:" << std::endl;
        errStream << msg;
}

void ErrorManager::newWarning(std::string msg) {
        errStream << "\033[1;33mwarning\033[0m:" << std::endl;
        errStream << msg;
}

bool ErrorManager::getErrors() const { return errors; }

void ErrorManager::report() {
        std::string messages = errStream.str();
        if (messages.length() > 0)
                std::cerr << errStream.str() << std::endl;
}
