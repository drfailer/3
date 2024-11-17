#ifndef TYPES_H
#define TYPES_H
#include <iostream>
#include <list>
#include <map>
#include <memory>
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
enum class TypeKinds { Primitive, StaticArray, DynamicArray, Obj, Function };

struct Type {
    Type(TypeKinds kind) : kind(kind) {}
    virtual ~Type() {}
    virtual std::string toString() const = 0;
    virtual PrimitiveTypes getEvaluatedType() const = 0;
    virtual bool compare(std::shared_ptr<Type> const) const = 0;
    TypeKinds kind;
};

using type = std::shared_ptr<Type>;
using types = std::list<type>;

template <typename T, typename... Args>
std::shared_ptr<T> make_type(Args &&...args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

std::ostream &operator<<(std::ostream &os, PrimitiveTypes type);
std::ostream &operator<<(std::ostream &os, types const &types);

struct Primitive : Type {
    Primitive(PrimitiveTypes type = INT)
        : Type(TypeKinds::Primitive), type(type) {}
    PrimitiveTypes type;

    std::string toString() const override {
        std::ostringstream oss;
        oss << type;
        return oss.str();
    }

    PrimitiveTypes getEvaluatedType() const override { return type; }

    bool compare(std::shared_ptr<Type> const other) const override {
        if (other->kind != TypeKinds::Primitive) {
            return false;
        }
        auto otherPrimitive = std::static_pointer_cast<Primitive>(other);
        return otherPrimitive->type == type;
    }
};

struct StaticArray : Type {
    StaticArray(PrimitiveTypes elementType = INT, size_t size = 0)
        : Type(TypeKinds::StaticArray), elementType(elementType), size(size) {}
    PrimitiveTypes elementType;
    size_t size;

    std::string toString() const override {
        std::ostringstream oss;
        oss << elementType << "[" << size << "]";
        return oss.str();
    }

    PrimitiveTypes getEvaluatedType() const override { return elementType; }

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
    DynamicArray(PrimitiveTypes elementType)
        : Type(TypeKinds::DynamicArray), elementType(elementType) {}
    PrimitiveTypes elementType;

    std::string toString() const override {
        std::ostringstream oss;
        oss << elementType << "[dyn]";
        return oss.str();
    }

    PrimitiveTypes getEvaluatedType() const override { return elementType; }

    bool compare(std::shared_ptr<Type> const other) const override {
        if (other->kind != TypeKinds::DynamicArray) {
            return false;
        }
        auto otherDynamicArray = std::static_pointer_cast<DynamicArray>(other);
        return otherDynamicArray->elementType == elementType;
    }
};

struct Obj : Type {
    Obj(std::string const &name, std::map<std::string, type> const &types)
        : Type(TypeKinds::Obj), name(name), types(types) {}
    std::string name;
    std::map<std::string, type> types = {};

    std::string toString() const override { return name; }

    PrimitiveTypes getEvaluatedType() const override {
        throw std::runtime_error("todo: how to handle returned structs");
        return NIL;
    }
};

struct Function : Type {
    Function(type returnType = make_type<Primitive>(NIL),
             types const &argumentsTypes = {})
        : Type(TypeKinds::Function), returnType(returnType),
          argumentsTypes(argumentsTypes) {}
    type returnType = nullptr;
    types argumentsTypes = {};

    std::string toString() const override {
        std::ostringstream oss;
        oss << returnType->toString() << argumentsTypes;
        return oss.str();
    }

    PrimitiveTypes getEvaluatedType() const override {
        return returnType->getEvaluatedType();
    }

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

type selectType(type left, type right);
bool isArray(type t);
bool isArrayOfChr(type const t);
size_t getArraySize(type const t);
PrimitiveTypes getElementType(type const t);
PrimitiveTypes getValueType(type const t);
bool isNumber(type const t);
type getReturnType(type const t);

} // end namespace type_system

#endif
