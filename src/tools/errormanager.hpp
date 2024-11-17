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
                             type_system::types expected,
                             type_system::types found);
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
                                std::string functionName,
                                type_system::type expected,
                                type_system::type found);
    void addReturnTypeWarning(std::string file, int line, std::string name,
                              type_system::type expected,
                              type_system::type found);

  private:
    std::ostringstream errStream;
    bool errors;
};

#endif
