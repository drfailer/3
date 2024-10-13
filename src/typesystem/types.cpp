#include "types.hpp"
#include <iostream>
#include <list>

std::ostream &operator<<(std::ostream &os, const PrimitiveType &type) {
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

std::ostream &operator<<(std::ostream &os, const std::list<PrimitiveType> types) {
    for (PrimitiveType type : types) {
        os << type << " -> ";
    }
    os << ".";
    return os;
}

PrimitiveType getArrayType(PrimitiveType type) {
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

PrimitiveType getValueType(PrimitiveType type) {
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

bool isArray(PrimitiveType type) {
    return type == ARR_INT || type == ARR_FLT || type == ARR_CHR;
}

bool isNumber(PrimitiveType type) {
    PrimitiveType typeToCheck = getValueType(type);
    return typeToCheck == INT || typeToCheck == FLT;
}

/**
 * @brief  Automatic convertion to flt in operators (flt op int).
 */
PrimitiveType selectType(PrimitiveType left, PrimitiveType right) {
    PrimitiveType type;
    if (left == INT && right == INT) {
        type = INT;
    } else {
        type = FLT;
    }
    return type;
}
