#include "type.hpp"
#include <sstream>

#define new_type(type, value_name, ...)                                        \
    new Type {                                                                 \
        .kind = TypeKind::type, .value = {                                     \
            .value_name = new type##Type{__VA_ARGS__},                         \
        }                                                                      \
    }

namespace type {

Type *create_nil_type() {
    return new Type{TypeKind::Nil, {.function = nullptr}};
}

Type *create_primitive_type(PrimitiveType type) {
    return new Type{TypeKind::Primitive, {.primitive = type}};
}

Type *create_static_array_type(Type *type, size_t size) {
    return new_type(Array, array, type, size, false);
}
Type *create_dynamic_array_type(Type *type) {
    return new_type(Array, array, type, 0, true);
}

Type *create_obj_type(std::string const &id,
                      std::map<std::string, Type *> &&fields) {
    return new_type(Obj, obj, id, fields);
}

Type *create_function_type(std::string const &id, Type *return_type,
                           std::list<Type *> &&arguments_types) {
    return new_type(Function, function, id, return_type, arguments_types);
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
        break;
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

} // end namespace type

#undef new_type
