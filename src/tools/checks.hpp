#ifndef CHECKS_H
#define CHECKS_H
#include "ast/ast.hpp"
#include "symtable/contextmanager.hpp"
#include "tools/errormanager.hpp"

extern ContextManager contextManager;
extern ErrorManager errMgr;

bool isDefined(std::string file, int line, std::string name,
               std::list<PrimitiveType> &type);
void printType(std::ostringstream &oss, std::list<PrimitiveType> types);
bool checkTypeError(std::list<PrimitiveType> expectedType, std::list<PrimitiveType> funcallType);
void checkType(std::string file, int line, std::string name, PrimitiveType expected,
               PrimitiveType found);
std::list<PrimitiveType> getTypes(std::list<std::shared_ptr<TypedNode>> nodes);

#endif
