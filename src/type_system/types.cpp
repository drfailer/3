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
    case NIL:
        os << "nil";
        break;
    case STR:
        os << "str";
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

    // FIXME: could go wrong
    primitiveLeft = std::static_pointer_cast<Primitive>(left->getEvaluatedType())->type;
    primitiveRight = std::static_pointer_cast<Primitive>(right->getEvaluatedType())->type;
    if (primitiveLeft == INT && primitiveRight == INT) {
        return make_type<Function>(make_type<Primitive>(INT), arguments);
    }
    return make_type<Function>(make_type<Primitive>(FLT), arguments);
}

bool isArray(type_t const type) {
    return type->kind == TypeKinds::StaticArray ||
           type->kind == TypeKinds::DynamicArray;
}

bool isPrimitive(type_t const type) {
    return type->kind == TypeKinds::Primitive;
}

size_t getArraySize(type_t const type) {
    auto arrayType = std::static_pointer_cast<type_system::StaticArray>(type);

    if (!arrayType) {
        throw std::runtime_error("error: try to use getArraySize on a type "
                                 "that is not a static array.");
    }
    return arrayType->size;
}

std::optional<PrimitiveTypes> getPrimitiveType(type_t const type) {
    if (auto primitive = std::static_pointer_cast<Primitive>(type)) {
        return std::make_optional<PrimitiveTypes>(primitive->type);
    }
    return std::nullopt;
}

std::shared_ptr<Type> getElementType(type_t const type) {
    switch (type->kind) {
    case TypeKinds::StaticArray: {
        auto staticArray = std::static_pointer_cast<StaticArray const>(type);
        return staticArray->elementType;
    } break;
    case TypeKinds::DynamicArray: {
        auto dynamicArray = std::static_pointer_cast<DynamicArray const>(type);
        return dynamicArray->elementType;
    } break;
    default:
        return type;
        break;
        ;
    }
    return make_type<Primitive>(NIL);
}

bool isArrayOfChr(type_t const type) {
    if (auto primitive = getPrimitiveType(getElementType(type))) {
        return primitive.value() == CHR;
    }
    return false;
}

bool isLiteralString(type_t const type) {
    if (auto primitive = std::dynamic_pointer_cast<Primitive>(type)) {
        return primitive->type == STR;
    }
    return false;
}

bool isNumber(type_t const type) {
    if (auto evaluatedType = getPrimitiveType(type->getEvaluatedType())) {
        return evaluatedType.value() == INT || evaluatedType.value() == FLT;
    }
    return false;
}

bool isNone(type_t const type) {
    return type->kind == TypeKinds::None;
}

bool isNil(type_t const type) {
    if (auto primitive = getPrimitiveType(type)) {
        return primitive.value() == PrimitiveTypes::NIL;
    }
    return false;
}

bool isCastableTo(type_t const t1, type_t const t2) {
    auto p1 = getPrimitiveType(t1);
    auto p2 = getPrimitiveType(t2);

    if (p1.has_value() && p2.has_value()) {
        switch (p1.value()) {
            case PrimitiveTypes::INT:
                return p2.value() == PrimitiveTypes::FLT ||
                       p2.value() == PrimitiveTypes::INT ||
                       p2.value() == PrimitiveTypes::CHR;
                break;
            case PrimitiveTypes::CHR:
                return p2.value() == PrimitiveTypes::INT ||
                       p2.value() == PrimitiveTypes::CHR;
                break;
            case PrimitiveTypes::FLT:
                return p2.value() == PrimitiveTypes::INT;
                break;
            default:
                break;
        }
    }
    return false;
}

type_t getReturnType(type_t const type) {
    switch (type->kind) {
    case TypeKinds::Function: {
        auto functionType = std::static_pointer_cast<Function>(type);
        return functionType->returnType;
    } break;
    default:
        break;
    }
    return make_type<Primitive>(NIL);
}

size_t getPrimitiveTypeSize(type_t const type) {
    if (auto primitive = std::dynamic_pointer_cast<Primitive>(type)) {
        return primitive->size;
    }
    return 0;
}


} // end namespace type_system
