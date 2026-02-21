#ifndef TYPE
#define TYPE
#include "tools/string.hpp"
#include "tools/array.hpp"
#include "tools/mem.hpp"

struct Type;

// Numbers in the struct fields are used to compare types when selecting
// arithmetic expression result type (no cast for now so we select
// automatically).
enum class PrimitiveType {
    // Bol, // TODO: implement the bool type
    Chr = 0,
    Int = 1,
    Flt = 2,
    Str = 3,
};

struct ArrayType {
    Type *element_type;
    size_t size; // size for static array
    bool dynamic;
};

struct ObjTypeField {
    String name;
    Type  *type;
};

struct ObjType {
    // String id; // ?
    Array<ObjTypeField> fields;
};

struct FunctionType {
    Type *return_type;
    Array<Type*> arguments_types;
};

enum class TypeKind {
    Nil,
    Primitive,
    Array,
    Obj,
    Function,
};

union TypeData {
    PrimitiveType primitive;
    ArrayType array;
    ObjType obj;
    FunctionType function;
};

struct Type {
    TypeKind kind;
    TypeData data;
};

inline Type *new_type_(MemPool<Type> *pool, TypeKind kind, TypeData data) {
    return mem_pool_alloc(pool, Type{
        .kind = kind,
        .data = data,
    });
}
#define new_type(pool, kind, ...) new_type_((pool), (kind), TypeData{__VA_ARGS__})

bool equal(Type const *t1, Type const *t2);
bool is_convertible(Type const *from, Type const *to);

bool is_primitive(Type const *type);
bool is_int(Type const *type);
bool is_chr(Type const *type);
bool is_flt(Type const *type);
bool is_str(Type const *type);
bool is_nil(Type const *type);

bool is_number(Type const *type);
bool supports_arithmetic(Type *type);
Type *select_most_precise_arithmetic_type(Type *lhs, Type *rhs);

size_t size_of(Type *type);

std::string type_to_string(Type *type);

#endif
