#ifndef __TYPES__
#define __TYPES__
#include <iostream>

union type_t
{
  long long i;
  double f;
  char c;
};

enum Type
{
  INT,
  FLT,
  CHR,
  VOID
};

std::string typeToString(Type type);

#endif
