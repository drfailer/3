#ifndef __TYPES__
#define __TYPES__
#include <iostream>
#include <list>
#define MAX_STRING_LENGTH 1000

union type_t {
        long long i;
        double f;
        char c;
        char s[MAX_STRING_LENGTH];
};

enum Type { INT, FLT, CHR, ARR_INT, ARR_FLT, ARR_CHR, VOID };

Type getArrayType(Type type);
Type getValueType(Type type);
bool isArray(Type type);

std::ostream& operator<<(std::ostream& os, const Type& type);
std::ostream& operator<<(std::ostream& os, const std::list<Type> types);

#endif
