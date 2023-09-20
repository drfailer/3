#include "Types.hpp"
#include <iostream>
#include <list>

std::ostream& operator<<(std::ostream& os, const Type& type) {
        switch (type) {
        case INT:
                os << "int";
                break;
        case FLT:
                os << "flt";
                break;
        case CHR:
                os << "chr";
                break;
        case ARR_INT:
                os << "int[]";
                break;
        case ARR_FLT:
                os << "flt[]";
                break;
        case ARR_CHR:
                os << "chr[]";
                break;
        default:
                os << "void";
                break;
        }
        return os;
}

std::ostream& operator<<(std::ostream& os, const std::list<Type> types) {
        for (Type type : types) {
                os << type << " -> ";
        }
        os << "." << std::endl;
        return os;
}

Type getArrayType(Type type) {
        switch (type) {
        case INT:
                return ARR_INT;
                break;
        case FLT:
                return ARR_FLT;
                break;
        case CHR:
                return ARR_CHR;
                break;
        default:
                return type;
                break;
        }
}

Type getValueType(Type type) {
        switch (type) {
        case ARR_INT:
                return INT;
                break;
        case ARR_FLT:
                return FLT;
                break;
        case ARR_CHR:
                return CHR;
                break;
        default:
                return type;
                break;
        }
}

bool isArray(Type type) {
        return type == ARR_INT || type == ARR_FLT || type == ARR_CHR;
}

bool isNumber(Type type) {
        Type typeToCheck = getValueType(type);
        return typeToCheck == INT || typeToCheck == FLT;
}

