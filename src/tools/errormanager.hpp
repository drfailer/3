#ifndef ERROR_MANAGER_H
#define ERROR_MANAGER_H
// this shouldn't be done this way -> add include directrories in the CMake
// configuration
#include "typesystem/types.hpp"
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
                             std::list<Type> expected, std::list<Type> found);
    void addUndefinedSymbolError(std::string file, int line, std::string name);
    void addMultipleDefinitionError(std::string file, int line,
                                    std::string name);
    void addUnexpectedReturnError(std::string file, int line,
                                  std::string functionName);
    void addBadArrayUsageError(std::string file, int line, std::string name);
    void addNoEntryPointError();
    void addOperatorError(std::string file, int line, std::string name);
    void addLiteralStringOverflowError(std::string file, int line);

    // Warnings:
    void addTypeAssignedWarning(std::string file, int line,
                                std::string functionName, Type expected,
                                Type found);
    void addReturnTypeWarning(std::string file, int line, std::string name,
                              Type expected, Type found);

  private:
    std::ostringstream errStream;
    bool errors;
};

#endif
