#include "checks.hpp"
#include <optional>

// TODO: rewrite all this stuff

// symtable check
// TODO: should return an optional
bool isDefined(std::string file, int line, std::string name,
               type_system::type &type) {
    bool defined = true;
    std::optional<Symbol> sym = contextManager.lookup(name);

    if (!sym.has_value()) {
        // TODO: this should be not in the errMgr, not here
        defined = false;
        errMgr.addUndefinedSymbolError(file, line, name);
        type = {};
    } else {
        type = sym.value().getType();
    }
    return defined;
}

bool checkParametersTypes(type_system::types expectedTypes,
                          type_system::types funcallTypes) {
    auto expectedIt = expectedTypes.begin(), expectedEnd = expectedTypes.end();
    auto funcallIt = funcallTypes.begin(), funcallEnd = funcallTypes.begin();

    while (expectedIt != expectedEnd && funcallIt != funcallEnd) {
        if (!(*expectedIt++)->compare(*funcallIt++)) {
            return false;
        }
    }
    return expectedIt == expectedEnd && funcallIt == funcallEnd;
}

void checkType(std::string file, int line, std::string name,
               type_system::type expected, type_system::type found) {
    if (found == nullptr || expected == nullptr) {
        return;
    }

    if (!expected->compare(found)) {
        errMgr.addTypeAssignedWarning(file, line, name, expected, found);
    }
}

void checkType(std::string file, int line, std::string name,
               type_system::type expected, type_system::PrimitiveTypes found) {
    auto primitiveFound = type_system::make_type<type_system::Primitive>(found);
    checkType(file, line, name, expected, primitiveFound);
}
