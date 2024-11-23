#include "checks.hpp"
#include <optional>

// TODO: rewrite all this stuff

// symtable check
// TODO: should return an optional
bool isDefined(ProgramBuilder &pb, std::string file, int line, std::string name,
               type_system::type &type) {
    bool defined = true;
    std::optional<Symbol> sym = pb.contextManager().lookup(name);

    if (!sym.has_value()) {
        // TODO: this should be not in the pb.errMgr(), not here
        defined = false;
        pb.errMgr().addUndefinedSymbolError(file, line, name);
        type = {};
    } else {
        type = sym.value().getType();
    }
    return defined;
}

bool checkParametersTypes(ProgramBuilder &, type_system::types expectedTypes,
                          type_system::types funcallTypes) {
    auto expectedIt = expectedTypes.begin(), expectedEnd = expectedTypes.end();
    auto funcallIt = funcallTypes.begin(), funcallEnd = funcallTypes.end();

    while (expectedIt != expectedEnd && funcallIt != funcallEnd) {
        if (!(*expectedIt++)->compare(*funcallIt++)) {
            return false;
        }
    }
    return expectedIt == expectedEnd && funcallIt == funcallEnd;
}

void checkType(ProgramBuilder &pb, std::string file, int line, std::string name,
               type_system::type expected, type_system::type found) {
    if (found == nullptr || expected == nullptr) {
        return;
    }

    if (!expected->compare(found)) {
        pb.errMgr().addTypeAssignedWarning(file, line, name, expected, found);
    }
}

void checkType(ProgramBuilder &pb, std::string file, int line, std::string name,
               type_system::type expected, type_system::PrimitiveTypes found) {
    auto primitiveFound = type_system::make_type<type_system::Primitive>(found);
    checkType(pb, file, line, name, expected, primitiveFound);
}
