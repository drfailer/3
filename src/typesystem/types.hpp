#ifndef TYPES_H
#define TYPES_H
#include <iostream>
#include <list>
#define MAX_LITERAL_STRING_LENGTH 1000

union LiteralValue {
        long long _int;
        double _flt;
        char _chr;
        char _str[MAX_LITERAL_STRING_LENGTH];
};

enum Type { INT, FLT, CHR, ARR_INT, ARR_FLT, ARR_CHR, VOID };

Type getArrayType(Type type);
Type getValueType(Type type);
bool isArray(Type type);
bool isNumber(Type type);

std::ostream& operator<<(std::ostream& os, const Type& type);
std::ostream& operator<<(std::ostream& os, const std::list<Type> types);

Type selectType(Type left, Type right);

#endif
