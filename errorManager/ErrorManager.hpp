#ifndef __ERROR_MANAGER__
#define __ERROR_MANAGER__
// this shouldn't be done this way -> add include directrories in the CMake
// configuration
#include "../AST/Types.hpp"
#include <iostream>
#include <sstream>

class ErrorManager {
      public:
        void report();
        bool getErrors() const { return errors; }
        void addError(std::string message);
        void addWarning(std::string message);

        // Errors:
        void addFuncallTypeError(std::string name, int line, int column,
                          std::list<Type> expected, std::list<Type> found);
        void addUndefinedSymbolError(std::string name, int line, int column);
        void addMultipleDefinitionError(std::string name, int line, int column);
        void addUnexpectedReturnError(std::string functionName, int line,
                                      int column);
        void addBadArrayUsageError(std::string name, int line, int column);
        void addNoEntryPointError();

        // Warnings:
        void addTypeAssignedWarning(std::string functionName, int line, int column,
                            Type expected, Type found);
        void addReturnTypeWarning(std::string name, int line, int column,
                                  Type expected, Type found);

        ErrorManager() : errors(false) {}
        ~ErrorManager() = default;

      private:
        std::ostringstream errStream;
        bool errors;
};

#endif
