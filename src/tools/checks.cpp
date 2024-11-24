#include "checks.hpp"
#include "tools/s3c.hpp"
#include <optional>

// TODO: rewrite all this stuff

// symtable check
// TODO: should return an optional
bool isDefined(S3C &s3c, std::string file, int line, std::string name,
               type_system::type &type) {
    bool defined = true;
    std::optional<Symbol> sym = s3c.contextManager().lookup(name);

    if (!sym.has_value()) {
        // TODO: this should be not in the s3c.errorsManager(), not here
        defined = false;
        s3c.errorsManager().addUndefinedSymbolError(file, line, name);
        type = {};
    } else {
        type = sym.value().getType();
    }
    return defined;
}

bool checkParametersTypes(S3C &, type_system::types expectedTypes,
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

void checkType(S3C &s3c, std::string file, int line, std::string name,
               type_system::type expected, type_system::type found) {
    if (found == nullptr || expected == nullptr) {
        return;
    }

    if (!expected->compare(found)) {
        s3c.errorsManager().addTypeAssignedWarning(file, line, name, expected, found);
    }
}

void checkType(S3C &s3c, std::string file, int line, std::string name,
               type_system::type expected, type_system::PrimitiveTypes found) {
    auto primitiveFound = type_system::make_type<type_system::Primitive>(found);
    checkType(s3c, file, line, name, expected, primitiveFound);
}
