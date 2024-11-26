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

std::ostream &operator<<(std::ostream &os, types_t const &types) {
    auto it = types.cbegin();

    os << "(";
    if (it != types.cend()) {
        os << (*it++)->toString();
    }
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
type_t selectType(type_t left, type_t right) {
    PrimitiveTypes primitiveLeft, primitiveRight;
    type_t result;
    types_t arguments = {left, right};

    primitiveLeft = left->getEvaluatedType();
    primitiveRight = right->getEvaluatedType();
    if (primitiveLeft == INT && primitiveRight == INT) {
        return make_type<Function>(make_type<Primitive>(INT), arguments);
    }
    return make_type<Function>(make_type<Primitive>(FLT), arguments);
}

bool isArray(type_t t) {
    return t->kind == TypeKinds::StaticArray ||
           t->kind == TypeKinds::DynamicArray;
}

size_t getArraySize(type_t const t) {
    auto arrayType = std::static_pointer_cast<StaticArray>(t);

    if (!arrayType) {
        throw std::runtime_error("error: try to use getArraySize on a type "
                                 "that is not a static array.");
    }
    return arrayType->size;
}

// todo: array might store any type in the future
PrimitiveTypes getElementType(type_t const t) {
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

bool isArrayOfChr(type_t const t) { return getElementType(t) == CHR; }

bool isNumber(type_t const t) {
    PrimitiveTypes evaluatedType = t->getEvaluatedType();
    return evaluatedType == INT || evaluatedType == FLT;
}

bool isNone(type_t const t) {
    return t->kind == TypeKinds::None;
}

type_t getReturnType(type_t const t) {
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
