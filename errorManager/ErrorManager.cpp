#include "ErrorManager.hpp"
#include <ostream>
#define ERR "\033[1;31m"
#define WARN "\033[1;33m"
#define BOLD "\033[1;34m"
#define NORM "\033[0m"

/* TODO: the current system doesn't know the file where the error has been
 *       found.
 */

/**
 * @brief  Record a new error.
 * @param  msg  Error message.
 */
void ErrorManager::addError(std::string msg) {
        errors = true;
        errStream << ERR << "error" << NORM << ":" << std::endl;
        errStream << msg;
}

/**
 * @brief  Record a new warning.
 * @param  msg  Warning message.
 */
void ErrorManager::addWarning(std::string msg) {
        errStream << WARN << "warning" << NORM << ":" << std::endl;
        errStream << msg;
}

/**
 * @brief  Report all the warning and messages recorded.
 */
void ErrorManager::report() {
        std::string messages = errStream.str();
        if (messages.length() > 0) {
                std::cerr << errStream.str() << std::endl;
        }
}

/******************************************************************************/
/*                                   errors                                   */
/******************************************************************************/

/**
 * @brief  Record a funcall type error.
 *
 * @param  name      Name of the funcion.
 * @param  line      Location.line
 * @param  column    Location.column
 * @param  expected  Expected type (type of the function).
 * @param  found     Found type.
 */
void ErrorManager::addFuncallTypeError(std::string name, int line, int column,
                                       std::list<Type> expected,
                                       std::list<Type> found) {
        std::ostringstream oss;
        oss << "Type error: at " << line << ":" << column << " in " << BOLD
            << name << NORM << ": the expected type was:" << std::endl;
        oss << BOLD << expected << NORM;
        oss << "found:" << std::endl;
        oss << BOLD << found << NORM;
        addError(oss.str());
}

/**
 * @brief  Record a multiple definition erro. This occurs when we try to
 *         overload a function.
 *
 * @param  name      Name of the function.
 * @param  line      Location.line
 * @param  column    Location.column
 */
void ErrorManager::addMultipleDefinitionError(std::string name, int line,
                                              int column) {
        std::ostringstream oss;
        oss << "redefinition of " << BOLD << name << NORM << " at " << line
            << ":" << column << "." << std::endl;
        addError(oss.str());
}

/**
 * @brief  Record an unexpected return error. This occurs when we try to return
 *         a value inside a procedure (void function).
 *
 * @param  name      Name of the procedure.
 * @param  line      Location.line
 * @param  column    Location.column
 */
void ErrorManager::addUnexpectedReturnError(std::string functionName, int line,
                                            int column) {
        std::ostringstream oss;
        oss << "found return statement in " << BOLD << functionName << NORM
            << " at " << line << ":" << column << " which is of type void"
            << std::endl;
        addError(oss.str());
}

/**
 * @brief  Record a bad array usage error. This occurs when we try to use a
 *         standard variable like an array:
 *         int a;
 *         set(a[0], 1);
 *         NOTE: there is currently no pointers in the language so arrays are
 *               treated like special elements.
 *
 * @param  name      Name of the variable.
 * @param  line      Location.line
 * @param  column    Location.column
 */
void ErrorManager::addBadArrayUsageError(std::string name, int line,
                                         int column) {
        std::ostringstream oss;
        oss << BOLD << name << NORM << " can't be used as an array at " << line
            << ":" << column << "." << std::endl;
        addError(oss.str());
}

/**
 * @brief  Record an undefined symbol error.
 *
 * @param  name      Name of the symbol.
 * @param  line      Location.line
 * @param  column    Location.column
 */
void ErrorManager::addUndefinedSymbolError(std::string name, int line,
                                           int column) {
        std::ostringstream oss;
        oss << "undefined Symbol " << BOLD << name << NORM;
        oss << " at: " << line << ":" << column << std::endl;
        addError(oss.str());
}

/**
 * @brief  Record an operator error. This occurs when an operator is used and
 *         lhs or rhs has the wrong type (for instance: `add(1, 'a')`).
 *
 * @param  name      Name of the oporator.
 * @param  line      Location.line
 * @param  column    Location.column
 */
void ErrorManager::addOperatorError(std::string name, int line, int column) {
        std::ostringstream oss;
        oss << "bad usage of operator " << BOLD << name << NORM;
        oss << " at " << line << ":" << column << std::endl;
        addError(oss.str());
}

/**
 * @brief  Record a no entry point error. This occurs when no main function has
 *         been found.
 */
void ErrorManager::addNoEntryPointError() { addError("no entry point."); }

/******************************************************************************/
/*                                  warnings                                  */
/******************************************************************************/

/**
 * @brief  Record an assigned type warning. This occurs when we assign value to
 *         a variable that has a different type.
 *
 * @param  name      Name of the variable.
 * @param  line      Location.line
 * @param  column    Location.column
 * @param  expected  Expected type (type of the variable).
 * @param  found     Found type (type of the value).
 */
void ErrorManager::addTypeAssignedWarning(std::string name, int line,
                                          int column, Type expected,
                                          Type found) {
        std::ostringstream oss;
        oss << "assignment at " << line << ":" << column << " " << BOLD << name
            << NORM << " is of type " << BOLD << expected
            << NORM " but the value assigned is of type " << BOLD << found
            << NORM << "." << std::endl;
        addWarning(oss.str());
}

/**
 * @brief  Record a return type warning. This occurs when the type of the
 *         returned value doesn't match the return type of the function.
 *         This is a warning and not an error as the value can be casted.
 *
 * @param  name      Name of the funcion.
 * @param  line      Location.line
 * @param  column    Location.column
 * @param  expected  Expected return type.
 * @param  found     Found return type.
 */
void ErrorManager::addReturnTypeWarning(std::string functionName, int line,
                                        int column, Type expected, Type found) {
        std::ostringstream oss;
        oss << "in " << BOLD << functionName << NORM << " at " << line << ":"
            << column << " return of type " << BOLD << expected << NORM
            << " but this function is of type " << BOLD << found << NORM << "."
            << std::endl;
        addWarning(oss.str());
}
