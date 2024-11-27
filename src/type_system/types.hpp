#ifndef TYPES_H
#define TYPES_H
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#define MAX_LITERAL_STRING_LENGTH 1000

namespace type_system {

union LiteralValue {
    long long _int;
    double _flt;
    char _chr;
    char _str[MAX_LITERAL_STRING_LENGTH];
};

enum PrimitiveTypes { NIL, INT, FLT, CHR };
enum class TypeKinds {
    Primitive,
    StaticArray,
    DynamicArray,
    Obj,
    Function,
    None
};

struct Type {
    Type(TypeKinds kind) : kind(kind) {}
    virtual ~Type() {}
    virtual std::string toString() const = 0;
    virtual std::shared_ptr<Type> const getEvaluatedType() const = 0;
    virtual bool compare(std::shared_ptr<Type> const) const = 0;
    TypeKinds kind;
};

using type_t = std::shared_ptr<Type>;
using types_t = std::list<type_t>;

template <typename T, typename... Args>
std::shared_ptr<T> make_type(Args &&...args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

std::ostream &operator<<(std::ostream &os, PrimitiveTypes type);
std::ostream &operator<<(std::ostream &os, types_t const &types);

/*
 * None type used for elements that should be verified after the parser
 * termination.
 */
struct None : Type {
    None() : Type(TypeKinds::None) {}
    virtual std::string toString() const { return "none"; }

    virtual type_t const getEvaluatedType() const { return make_type<None>(); }

    virtual bool compare(std::shared_ptr<Type> const other) const {
        return other->kind == TypeKinds::None;
    }
};

struct Primitive : Type {
    Primitive(PrimitiveTypes type = INT)
        : Type(TypeKinds::Primitive), type(type) {}
    PrimitiveTypes type;

    std::string toString() const override {
        std::ostringstream oss;
        oss << type;
        return oss.str();
    }

    type_t const getEvaluatedType() const override {
        return make_type<Primitive>(type);
    }

    bool compare(std::shared_ptr<Type> const other) const override {
        if (other->kind != TypeKinds::Primitive) {
            return false;
        }
        auto otherPrimitive = std::static_pointer_cast<Primitive>(other);
        return otherPrimitive->type == type;
    }
};

struct StaticArray : Type {
    // FIXME: should take a type as parameter
    StaticArray(type_t type, size_t size = 0)
        : Type(TypeKinds::StaticArray), elementType(type), size(size) {}
    type_t elementType;
    size_t size;

    std::string toString() const override {
        std::ostringstream oss;
        oss << elementType << "[" << size << "]";
        return oss.str();
    }

    // FIXME: it is not elementType
    type_t const getEvaluatedType() const override { return elementType; }

    bool compare(std::shared_ptr<Type> const other) const override {
        if (other->kind != TypeKinds::StaticArray) {
            return false;
        }
        auto otherStaticArray = std::static_pointer_cast<StaticArray>(other);
        return otherStaticArray->elementType == elementType &&
               otherStaticArray->size == size;
    }
};

struct DynamicArray : Type {
    // FIXME: should take a type as parameter
    DynamicArray(type_t type)
        : Type(TypeKinds::DynamicArray), elementType(type) {}
    type_t elementType;

    std::string toString() const override {
        std::ostringstream oss;
        oss << elementType << "[dyn]";
        return oss.str();
    }

    // FIXME: it is not elementType
    type_t const getEvaluatedType() const override { return elementType; }

    bool compare(std::shared_ptr<Type> const other) const override {
        if (other->kind != TypeKinds::DynamicArray) {
            return false;
        }
        auto otherDynamicArray = std::static_pointer_cast<DynamicArray>(other);
        return otherDynamicArray->elementType == elementType;
    }
};

struct Obj : Type {
    Obj(std::string const &name, std::map<std::string, type_t> const &types)
        : Type(TypeKinds::Obj), name(name), types(types) {}
    std::string name;
    std::map<std::string, type_t> types = {};

    std::string toString() const override { return name; }

    type_t const getEvaluatedType() const override {
        throw std::runtime_error("todo: how to handle returned structs");
        return nullptr;
    }
};

struct Function : Type {
    Function(type_t returnType = make_type<Primitive>(NIL),
             types_t const &argumentsTypes = {})
        : Type(TypeKinds::Function), returnType(returnType),
          argumentsTypes(argumentsTypes) {}
    type_t returnType = nullptr;
    types_t argumentsTypes = {};

    std::string toString() const override {
        std::ostringstream oss;
        oss << returnType->toString() << argumentsTypes;
        return oss.str();
    }

    type_t const getEvaluatedType() const override { return returnType; }

    bool compare(std::shared_ptr<Type> const other) const override {
        if (other->kind != TypeKinds::Function) {
            return false;
        }
        auto otherFunction = std::static_pointer_cast<Function>(other);
        if (!otherFunction->returnType->compare(returnType)) {
            return false;
        }

        auto thisIt = argumentsTypes.cbegin();
        auto thisEnd = argumentsTypes.cend();
        auto otherIt = otherFunction->argumentsTypes.cbegin();
        auto otherEnd = otherFunction->argumentsTypes.cend();
        for (; otherIt != otherEnd; otherIt++) {
            if (thisIt == thisEnd || !(*thisIt++)->compare(*otherIt)) {
                return false;
            }
        }

        return true;
    }
};

type_t selectType(type_t left, type_t right);
bool isArray(type_t type);
bool isArrayOfChr(type_t const type);
size_t getArraySize(type_t const type);
std::optional<PrimitiveTypes> getPrimitiveType(type_t const type);
std::shared_ptr<Type> getElementType(type_t const type);
PrimitiveTypes getValueType(type_t const type);
bool isNumber(type_t const type);
bool isNone(type_t const type);
bool isNil(type_t const type);
bool isCastableTo(type_t const t1, type_t const t2);
type_t getReturnType(type_t const type);

} // end namespace type_system

#endif
