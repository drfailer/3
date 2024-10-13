#include "checks.hpp"
#include <optional>

// TODO: rewrite all this stuff

// symtable check
bool isDefined(std::string file, int line, std::string name,
               std::list<PrimitiveType> &type) {
        bool defined = true;
        std::optional<Symbol> sym = contextManager.lookup(name);

        if (!sym.has_value()) {
                // TODO: this should be not in the errMgr, not here
                defined = false;
                errMgr.addUndefinedSymbolError(file, line, name);
                type = std::list<PrimitiveType>();
        } else {
                type = sym.value().getType();
        }
        return defined;
}

bool checkTypeError(std::list<PrimitiveType> expectedType, std::list<PrimitiveType> funcallType) {
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

void checkType(std::string file, int line, std::string name, PrimitiveType expected,
               PrimitiveType found) {
        if (found == NIL || expected == NIL) {
                return;
        }

        if (expected != found) {
                errMgr.addTypeAssignedWarning(file, line, name, expected,
                                              found);
        }
}

// permet de récupérer les types des paramètres lors des appels de fonctions
std::list<PrimitiveType> getTypes(std::list<std::shared_ptr<TypedNode>> nodes) {
        std::list<PrimitiveType> types;
        for (std::shared_ptr<TypedNode> node : nodes) {
                types.push_back(node->type());
        }
        return types;
}
