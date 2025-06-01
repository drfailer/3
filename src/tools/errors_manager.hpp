#ifndef ERROR_MANAGER_H
#define ERROR_MANAGER_H
// this shouldn't be done this way -> add include directrories in the CMake
// configuration
#include "symtable/type.hpp"
#include <sstream>

class ErrorManager {
  public:
    ErrorManager() : errors(false) {}
    ~ErrorManager() = default;

    void report();
    bool getErrors() const { return errors; }
    void addError(std::string message);
    void addWarning(std::string message);

    // Errors:
    void addFuncallTypeError(std::string file, int line, std::string name,
                             type::Type *expected, type::Type *found);
    void addUndefinedSymbolError(std::string file, int line, std::string name);
    void addMultipleDefinitionError(std::string file, int line,
                                    std::string name);
    void addUnexpectedReturnError(std::string file, int line,
                                  std::string functionName);
    void addBadArrayUsageError(std::string file, int line, std::string name);
    void addNoEntryPointError();
    void addOperatorError(std::string file, int line, std::string name);
    void addLiteralStringOverflowError(std::string file, int line);
    void addReturnTypeError(std::string file, int line, std::string name,
                            type::Type *expected, type::Type *found);
    void addBadUsageOfShwError(std::string file, int line,
                               type::Type *exprType);

    // Warnings:
    void addTypeAssignedWarning(std::string file, int line,
                                std::string functionName, type::Type *expected,
                                type::Type *found);
    void addReturnTypeWarning(std::string file, int line, std::string name,
                              type::Type *expected, type::Type *found);

  private:
    std::ostringstream errStream;
    bool errors;
};

#endif
