#ifndef __ERROR_MANAGER__
#define __ERROR_MANAGER__
// this shouldn't be done this way -> add include directrories in the CMake
// configuration
#include "ast/Types.hpp"
#include <iostream>
#include <sstream>

class ErrorManager {
      public:
        void report();
        bool getErrors() const { return errors; }
        void addError(std::string message);
        void addWarning(std::string message);

        // Errors:
        void addFuncallTypeError(std::string file, int line, std::string name,
                                 std::list<Type> expected,
                                 std::list<Type> found);
        void addUndefinedSymbolError(std::string file, int line,
                                     std::string name);
        void addMultipleDefinitionError(std::string file, int line,
                                        std::string name);
        void addUnexpectedReturnError(std::string file, int line,
                                      std::string functionName);
        void addBadArrayUsageError(std::string file, int line,
                                   std::string name);
        void addNoEntryPointError();
        void addOperatorError(std::string file, int line, std::string name);

        // Warnings:
        void addTypeAssignedWarning(std::string file, int line,
                                    std::string functionName, Type expected,
                                    Type found);
        void addReturnTypeWarning(std::string file, int line, std::string name,
                                  Type expected, Type found);

        ErrorManager() : errors(false) {}
        ~ErrorManager() = default;

      private:
        std::ostringstream errStream;
        bool errors;
};

#endif
