#include "type.hpp"
#include <cassert>
#include <sstream>

bool equal(Type const *t1, Type const *t2) {
    if (t1->kind != t2->kind) {
        return false;
    }

    switch (t1->kind) {
    case TypeKind::Nil:
        return false;
    case TypeKind::Primitive:
        return t1->data.primitive == t2->data.primitive;
        break;
    case TypeKind::Array:
        return t1->data.array.dynamic == t2->data.array.dynamic &&
               t1->data.array.size == t2->data.array.size &&
               equal(t1->data.array.element_type, t2->data.array.element_type);
        break;
    case TypeKind::Obj: return false;
    case TypeKind::Function: return false;
    }
    return false; // unreachable
}

bool is_convertible(Type const *from, Type const *to) {
    if (from->kind == TypeKind::Array) {
        if (to->kind != TypeKind::Array) {
            return false;
        }
        return is_convertible(from->data.array.element_type,
                                   to->data.array.element_type);
    } else if (from->kind == TypeKind::Primitive) {
        // TODO: this should require type cast
        if (from->data.primitive != PrimitiveType::Str) {
            return to->data.primitive != PrimitiveType::Str;
        }
        return true;
    }
    return false;
}

bool is_primitive(Type const *type) {
    return type->kind == TypeKind::Primitive;
}

bool is_int(Type const *type) {
    return is_primitive(type) && type->data.primitive == PrimitiveType::Int;
}

bool is_chr(Type const *type) {
    return is_primitive(type) && type->data.primitive == PrimitiveType::Chr;
}

bool is_flt(Type const *type) {
    return is_primitive(type) && type->data.primitive == PrimitiveType::Flt;
}

bool is_str(Type const *type) {
    return is_primitive(type) && type->data.primitive == PrimitiveType::Str;
}

bool is_nil(Type const *type) {
    return type->kind == TypeKind::Nil;
}

bool is_number(Type const *type) {
    if (is_primitive(type)) {
        return type->data.primitive == PrimitiveType::Int ||
               type->data.primitive == PrimitiveType::Flt;
    }
    return false;
}

bool supports_arithmetic(Type *type) {
    if (type == nullptr) {
        return false;
    }
    if (is_primitive(type)) {
        return type->data.primitive == PrimitiveType::Int ||
               type->data.primitive == PrimitiveType::Flt ||
               type->data.primitive == PrimitiveType::Chr;
    }
    return false;
}

// This function is used to select the most precise return type for arithmetic
// operations. This is required for now since the cast operator is not
// implemented, but it may be removed in the future if manual cast is enforced.
Type *select_most_precise_arithmetic_type(Type *lhs, Type *rhs) {
    assert(supports_arithmetic(lhs));
    assert(supports_arithmetic(rhs));

    if ((unsigned)lhs->data.primitive > (unsigned)rhs->data.primitive) {
        return lhs;
    }
    return rhs;
}

size_t size_of(Type *type) {
    switch (type->kind) {
    case TypeKind::Nil:
        return 0;
    case TypeKind::Primitive:
        if (type->data.primitive == PrimitiveType::Str) {
            return 16; // ptr + size
        } else {
            return 8; // all primitive types are on 8B for now
        }
        break;
    case TypeKind::Array:
        if (type->data.array.dynamic) {
            return 16; // ptr + size
        } else {
            return type->data.array.size *
                   size_of(type->data.array.element_type);
        }
        break;
    case TypeKind::Obj: {
        size_t ttl_size = 0;
        for (auto const &field : type->data.obj.fields) {
            ttl_size += size_of(field.type);
        }
    } break;
    case TypeKind::Function:
        return 8; // ptr size
    }
    return 8; // all types are on 8B for now
}

std::string primitive_type_to_string(PrimitiveType primitive) {
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
        break;
    case TypeKind::Primitive:
        oss << primitive_type_to_string(type->data.primitive);
        break;
    case TypeKind::Array:
        oss << type_to_string(type->data.array.element_type) << "["
            << (type->data.array.dynamic
                    ? "dyn"
                    : std::to_string(type->data.array.size))
            << "]";
        break;
    case TypeKind::Obj:
        oss << "obj { ";
        for (auto field : type->data.obj.fields) {
            oss << type_to_string(field.type) << " " << field.name.ptr << ", ";
        }
        oss << " }";
        break;
    case TypeKind::Function:
        oss << type_to_string(type->data.function.return_type) << " (";
        for (auto argument : type->data.function.arguments_types) {
            oss << type_to_string(argument) << " ";
        }
        oss << ")";
        break;
    }
    return oss.str();
}
