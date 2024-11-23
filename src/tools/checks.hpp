#ifndef CHECKS_H
#define CHECKS_H
#include "ast/ast.hpp"
#include "tools/programbuilder.hpp"

bool isDefined(ProgramBuilder &pb, std::string file, int line, std::string name,
               type_system::type &type);
void printType(std::ostringstream &oss, type_system::types types);
bool checkParametersTypes(ProgramBuilder &pb, type_system::types expectedTypes,
                          type_system::types funcallTypes);
void checkType(ProgramBuilder &pb, std::string file, int line, std::string name,
               type_system::type expected, type_system::type found);
void checkType(ProgramBuilder &pb, std::string file, int line, std::string name,
               type_system::type expected, type_system::PrimitiveTypes found);

#endif
