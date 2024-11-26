#include "checks.hpp"
#include "tools/s3c.hpp"
#include <optional>

// TODO: rewrite all this stuff

// symtable check
// TODO: should return an optional
bool isDefined(S3C &s3c, std::string file, int line, std::string name,
               type_system::type_t &type) {
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

bool checkParametersTypes(S3C &, type_system::types_t expectedTypes,
                          type_system::types_t funcallTypes) {
    auto expectedIt = expectedTypes.begin(), expectedEnd = expectedTypes.end();
    auto funcallIt = funcallTypes.begin(), funcallEnd = funcallTypes.end();
    // todo: refactor getEvaluatedType function
    auto evaluatedPrimitiveType =
        type_system::make_type<type_system::Primitive>(type_system::NIL);

    while (expectedIt != expectedEnd && funcallIt != funcallEnd) {
        evaluatedPrimitiveType->type = (*funcallIt)->getEvaluatedType();
        if (!(*expectedIt++)->compare(evaluatedPrimitiveType)) {
            return false;
        }
        funcallIt++;
    }
    return expectedIt == expectedEnd && funcallIt == funcallEnd;
}

void checkType(S3C &s3c, std::string file, int line, std::string name,
               type_system::type_t expected, type_system::type_t found) {
    if (found == nullptr || expected == nullptr) {
        return;
    }

    if (!expected->compare(found)) {
        s3c.errorsManager().addTypeAssignedWarning(file, line, name, expected, found);
    }
}

void checkType(S3C &s3c, std::string file, int line, std::string name,
               type_system::type_t expected, type_system::PrimitiveTypes found) {
    auto primitiveFound = type_system::make_type<type_system::Primitive>(found);
    checkType(s3c, file, line, name, expected, primitiveFound);
}
