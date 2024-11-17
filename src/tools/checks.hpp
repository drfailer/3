#ifndef CHECKS_H
#define CHECKS_H
#include "ast/ast.hpp"
#include "symtable/contextmanager.hpp"
#include "tools/errormanager.hpp"

extern ContextManager contextManager;
extern ErrorManager errMgr;

bool isDefined(std::string file, int line, std::string name,
               type_system::type &type);
void printType(std::ostringstream &oss, type_system::types types);
bool checkParametersTypes(type_system::types expectedTypes,
                          type_system::types funcallTypes);
void checkType(std::string file, int line, std::string name,
               type_system::type expected, type_system::type found);
void checkType(std::string file, int line, std::string name,
               type_system::type expected, type_system::PrimitiveTypes found);

#endif
