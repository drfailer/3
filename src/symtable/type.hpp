#ifndef SYMTABLE_TYPE
#define SYMTABLE_TYPE
#include <cstddef>
#include <string>
#include <map>
#include <list>

namespace type {

struct Type;

enum class PrimitiveType {
    Int,
    Flt,
    Chr,
    Str,
};

struct ArrayType {
    Type *type;
    size_t size;
    bool dynamic;
};

struct ObjType {
    std::string id;
    std::map<std::string, Type*> fields;
};

struct Function {
    std::string id;
    Type *return_type;
    std::list<Type*> arguments_types;
};

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
        Function *function;
    } value;
};

bool type_equal(Type *t1, Type *t2);
bool type_is_convertible(Type *from, Type *to);

bool is_number(Type *type);

// TODO
std::string type_to_string(Type *type);

} // end namespace type

#endif
