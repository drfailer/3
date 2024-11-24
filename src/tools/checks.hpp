#ifndef CHECKS_H
#define CHECKS_H
#include "type_system/types.hpp"

class S3C;

bool isDefined(S3C &s3c, std::string file, int line, std::string name,
               type_system::type &type);
void printType(std::ostringstream &oss, type_system::types types);
bool checkParametersTypes(S3C &s3c, type_system::types expectedTypes,
                          type_system::types funcallTypes);
void checkType(S3C &s3c, std::string file, int line, std::string name,
               type_system::type expected, type_system::type found);
void checkType(S3C &s3c, std::string file, int line, std::string name,
               type_system::type expected, type_system::PrimitiveTypes found);

#endif
