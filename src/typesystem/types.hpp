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

enum PrimitiveType { NIL, INT, FLT, CHR, ARR_INT, ARR_FLT, ARR_CHR, OBJ };
enum ArrayType { STATIC, DYNAMIC, NONE };

struct Type {
    PrimitiveType primitiveType = NIL; //< primitive type or composed type
    ArrayType arrayType = NONE;        //< static, dynamic array or none
    std::list<Type> types = {};        //< list of types for composed types
};

PrimitiveType getArrayType(PrimitiveType type);
PrimitiveType getValueType(PrimitiveType type);
bool isArray(PrimitiveType type);
bool isNumber(PrimitiveType type);

std::ostream &operator<<(std::ostream &os, const PrimitiveType &type);
std::ostream &operator<<(std::ostream &os, const std::list<PrimitiveType> types);

PrimitiveType selectType(PrimitiveType left, PrimitiveType right);

#endif
