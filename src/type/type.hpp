#ifndef SYMTABLE_TYPE
#define SYMTABLE_TYPE
#include <cstddef>
#include <string>
#include <map>
#include <vector>

namespace type {

struct Type;

Type *create_nil_type();

enum class PrimitiveType {
    Int,
    Flt,
    Chr,
    Str,
};
Type *create_primitive_type(PrimitiveType type);

struct ArrayType {
    Type *type;
    size_t size;
    bool dynamic;
};
Type *create_static_array_type(Type *type, size_t size);
Type *create_dynamic_array_type(Type *type);

struct ObjType {
    std::string id;
    std::map<std::string, Type*> fields;
};
Type *create_obj_type(std::string const &id,
                      std::map<std::string, Type *> &&fields);

struct FunctionType {
    std::string id;
    Type *return_type;
    std::vector<Type*> arguments_types;
};
Type *create_function_type(std::string const &id, Type *return_type,
                           std::vector<Type *> &&arguments_types);

enum class TypeKind {
    Nil,
    Primitive,
    Array,
    Obj,
    Function,
};

struct Type {
    TypeKind kind;
    union {
        PrimitiveType primitive;
        ArrayType *array;
        ObjType *obj;
        FunctionType *function;
    } value;
};

std::string type_to_string(Type *type);

size_t get_type_size(Type *type);

} // end namespace type

#endif
