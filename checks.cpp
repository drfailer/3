#include "checks.hpp"
#include <iostream>
#include <optional>

// TODO: rewrite all this stuff

// symtable check
bool isDefined(std::string name, int line, int column, std::list<Type> &type) {
        bool defined = true;
        std::optional<Symbol> sym = contextManager.lookup(name);

        if (!sym.has_value()) {
                // TODO: this should be not in the errMgr, not here
                defined = false;
                errMgr.addUndefinedSymbolError(name, line, column);
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
                for (int i = 0; i < funcallType.size(); ++i) {
                        if (expectedType.size() == 0 ||
                            funcallType.front() != expectedType.front())
                                typeError = true;
                        funcallType.pop_front();
                        expectedType.pop_front();
                }
        }
        return typeError;
}

void checkType(std::string name, int line, int column, Type expected,
               Type found) {
        if (found == VOID || expected == VOID) {
                return;
        }

        if (expected != found) {
                errMgr.addTypeAssignedWarning(name, line, column, expected, found);
        }
}

// permet de récupérer les types des paramètres lors des appels de fonctions
std::list<Type> getTypes(std::list<std::shared_ptr<TypedElement>> nodes) {
        std::list<Type> types;
        for (std::shared_ptr<TypedElement> node : nodes) {
                types.push_back(node->getType());
        }
        return types;
}
