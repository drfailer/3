#include "predicates.hpp"

namespace type {

bool equal(Type const *t1, Type const *t2) {
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
               equal(t1->value.array->type, t2->value.array->type);
        break;
    case TypeKind::Obj:
        return t1->value.obj->id == t2->value.obj->id;
        break;
    case TypeKind::Function:
        return false;
    }
    return false; // unreachable
}

bool is_convertible(Type const *from, Type const *to) {
    if (from->kind == TypeKind::Array) {
        if (to->kind != TypeKind::Array) {
            return false;
        }
        return is_convertible(from->value.array->type,
                                   to->value.array->type);
    } else if (from->kind == TypeKind::Primitive) {
        // TODO: this should require type cast
        if (from->value.primitive != PrimitiveType::Str) {
            return to->value.primitive != PrimitiveType::Str;
        }
        return true;
    }
    return false;
}

bool is_primitive(Type const *type) {
    return type->kind == TypeKind::Primitive;
}

bool is_int(Type const *type) {
    return is_primitive(type) && type->value.primitive == PrimitiveType::Int;
}

bool is_chr(Type const *type) {
    return is_primitive(type) && type->value.primitive == PrimitiveType::Chr;
}

bool is_flt(Type const *type) {
    return is_primitive(type) && type->value.primitive == PrimitiveType::Flt;
}

bool is_number(Type const *type) {
    if (is_primitive(type)) {
        return type->value.primitive == PrimitiveType::Int ||
               type->value.primitive == PrimitiveType::Flt;
    }
    return false;
}

bool supports_arithmetic(Type *type) {
    if (is_primitive(type)) {
        return type->value.primitive == PrimitiveType::Int ||
               type->value.primitive == PrimitiveType::Flt ||
               type->value.primitive == PrimitiveType::Chr;
    }
    return false;
}


} // end namespace type
