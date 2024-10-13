#ifndef CHECKS_H
#define CHECKS_H
#include "ast/ast.hpp"
#include "symtable/contextmanager.hpp"
#include "tools/errormanager.hpp"

extern ContextManager contextManager;
extern ErrorManager errMgr;

bool isDefined(std::string file, int line, std::string name,
               std::list<Type> &type);
void printType(std::ostringstream &oss, std::list<Type> types);
bool checkTypeError(std::list<Type> expectedType, std::list<Type> funcallType);
void checkType(std::string file, int line, std::string name, Type expected,
               Type found);
std::list<Type> getTypes(std::list<std::shared_ptr<TypedNode>> nodes);

#endif
