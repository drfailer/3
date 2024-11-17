#include "types.hpp"
#include <iostream>

namespace type_system {

std::ostream &operator<<(std::ostream &os, PrimitiveTypes type) {
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
    default:
        os << "nil";
        break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, types const &types) {
    auto it = types.cbegin();

    os << "(" << (*it++)->toString();
    for (; it != types.cend(); it++) {
        os << ", " << (*it)->toString();
    }
    return os << ")";
}

std::ostream &operator<<(std::ostream &os, Function const &type) {
    os << type.returnType << type.argumentsTypes;
    return os;
}

/**
 * @brief  Automatic convertion to flt in operators (flt op int).
 */
type selectType(type left, type right) {
    PrimitiveTypes primitiveLeft, primitiveRight;
    type result;
    types arguments = {left, right};

    primitiveLeft = left->getEvaluatedType();
    primitiveRight = right->getEvaluatedType();
    if (primitiveLeft == INT && primitiveRight == INT) {
        return make_type<Function>(make_type<Primitive>(INT), arguments);
    }
    return make_type<Function>(make_type<Primitive>(FLT), arguments);
}

bool isArray(type t) {
    return t->kind == TypeKinds::StaticArray ||
           t->kind == TypeKinds::DynamicArray;
}

size_t getArraySize(type const t) {
    auto arrayType = std::static_pointer_cast<StaticArray>(t);

    if (!arrayType) {
        throw std::runtime_error("error: try to use getArraySize on a type "
                                 "that is not a static array.");
    }
    return arrayType->size;
}

// todo: array might store any type in the future
PrimitiveTypes getElementType(type const t) {
    switch (t->kind) {
    case TypeKinds::Primitive: {
        auto primitive = std::static_pointer_cast<Primitive>(t);
        return primitive->type;
    } break;
    case TypeKinds::StaticArray: {
        auto staticArray = std::static_pointer_cast<StaticArray const>(t);
        return staticArray->elementType;
    } break;
    case TypeKinds::DynamicArray: {
        auto dynamicArray = std::static_pointer_cast<DynamicArray const>(t);
        return dynamicArray->elementType;
    } break;
    default:
        break;
        ;
    }
    return NIL;
}

bool isArrayOfChr(type const t) { return getElementType(t) == CHR; }

bool isNumber(type const t) {
    switch (t->kind) {
    case TypeKinds::Primitive: {
        auto primitiveType = std::static_pointer_cast<Primitive const>(t);
        return primitiveType->type == INT || primitiveType->type == FLT;
    } break;
    default:
        break;
        ;
    }
    return false;
}

type getReturnType(type const t) {
    switch (t->kind) {
    case TypeKinds::Function: {
        auto functionType = std::static_pointer_cast<Function>(t);
        return functionType->returnType;
    } break;
    default:
        break;
    }
    return make_type<Primitive>(NIL);
}

} // end namespace type_system
