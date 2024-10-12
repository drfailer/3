#include "checks.hpp"
#include <iostream>
#include <optional>

// TODO: rewrite all this stuff

// symtable check
bool isDefined(std::string file, int line, std::string name,
               std::list<Type> &type) {
        bool defined = true;
        std::optional<Symbol> sym = contextManager.lookup(name);

        if (!sym.has_value()) {
                // TODO: this should be not in the errMgr, not here
                defined = false;
                errMgr.addUndefinedSymbolError(file, line, name);
                type = std::list<Type>();
        } else {
                type = sym.value().getType();
        }
        return defined;
}

bool checkTypeError(std::list<Type> expectedType, std::list<Type> funcallType) {
        bool typeError = false;
        if (expectedType.size() != funcallType.size()) {
                typeError = true;
        } else {
                for (size_t i = 0; i < funcallType.size(); ++i) {
                        if (expectedType.size() == 0 ||
                            funcallType.front() != expectedType.front())
                                typeError = true;
                        funcallType.pop_front();
                        expectedType.pop_front();
                }
        }
        return typeError;
}

void checkType(std::string file, int line, std::string name, Type expected,
               Type found) {
        if (found == VOID || expected == VOID) {
                return;
        }

        if (expected != found) {
                errMgr.addTypeAssignedWarning(file, line, name, expected,
                                              found);
        }
}

// permet de récupérer les types des paramètres lors des appels de fonctions
std::list<Type> getTypes(std::list<std::shared_ptr<TypedNode>> nodes) {
        std::list<Type> types;
        for (std::shared_ptr<TypedNode> node : nodes) {
                types.push_back(node->type());
        }
        return types;
}
