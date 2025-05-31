#include "type.hpp"
#include <sstream>

namespace type {

bool type_equal(Type *t1, Type *t2) {
    if (t1->kind != t2->kind) {
        return false;
    }

    switch (t1->kind) {
    case TypeKind::Nil:
        return false;
    case TypeKind::Primitive:
        return t1->value.primitive == t2->value.primitive;
        break;
    case TypeKind::Array:
        return t1->value.array->dynamic == t2->value.array->dynamic &&
               t1->value.array->size == t2->value.array->size &&
               type_equal(t1->value.array->type, t2->value.array->type);
        break;
    case TypeKind::Obj:
        return t1->value.obj->id == t2->value.obj->id;
        break;
    case TypeKind::Function:
        return false;
    }
    return false; // unreachable
}

bool type_is_convertible(Type *from, Type *to) {
    if (from->kind == TypeKind::Array) {
        if (to->kind != TypeKind::Array) {
            return false;
        }
        return type_is_convertible(from->value.array->type,
                                   to->value.array->type);
    } else if (from->kind == TypeKind::Primitive) {
        // TODO: this should require type cast
        return from->value.primitive != PrimitiveType::Str &&
               to->value.primitive != PrimitiveType::Str;
    }
    return false;
}

std::string type_to_string(PrimitiveType primitive) {
    switch (primitive) {
    case PrimitiveType::Int:
        return "int";
    case PrimitiveType::Flt:
        return "flt";
    case PrimitiveType::Chr:
        return "chr";
    case PrimitiveType::Str:
        return "str";
    }
    return "none";
}

std::string type_to_string(Type *type) {
    std::string output;
    std::ostringstream oss;

    switch (type->kind) {
    case TypeKind::Nil:
        oss << "nil";
    case TypeKind::Primitive:
        oss << type_to_string(type->value.primitive);
        break;
    case TypeKind::Array:
        oss << type_to_string(type->value.array->type) << "["
            << (type->value.array->dynamic
                    ? "dyn"
                    : std::to_string(type->value.array->size))
            << "]";
        break;
    case TypeKind::Obj:
        oss << type->value.obj->id << " { ";
        for (auto field : type->value.obj->fields) {
            oss << field.first << ": " << type_to_string(field.second) << " ";
        }
        oss << " }";
        break;
    case TypeKind::Function:
        oss << type_to_string(type->value.function->return_type) << " "
            << type->value.function->id << "(";
        for (auto argument : type->value.function->arguments_types) {
            oss << type_to_string(argument) << " ";
        }
        oss << ")";
        break;
    }
    return oss.str();
}

bool is_number(Type *type) {
    if (type->kind == TypeKind::Primitive) {
        return type->value.primitive == PrimitiveType::Int ||
            type->value.primitive == PrimitiveType::Flt;
    }
    return false;
}

} // end namespace type
