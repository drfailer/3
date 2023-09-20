#ifndef CHECKS_HPP
#define CHECKS_HPP
#include "ast/ProgramBuilder.hpp"
#include "errorManager/ErrorManager.hpp"
#include "preprocessor/preprocessor.hpp"
#include "symtable/ContextManager.hpp"
#include "symtable/Symbol.hpp"
#include "symtable/Symtable.hpp"

extern ContextManager contextManager;
extern ErrorManager errMgr;

bool isDefined(std::string name, int line, int column, std::list<Type> &type);
void printType(std::ostringstream &oss, std::list<Type> types);
bool checkTypeError(std::list<Type> expectedType, std::list<Type> funcallType);
void checkType(std::string name, int line, int column, Type expected,
               Type found);
std::list<Type> getTypes(std::list<std::shared_ptr<TypedElement>> nodes);

#endif
