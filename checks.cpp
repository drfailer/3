#include "checks.hpp"
#include <iostream>
#include <optional>

// TODO: rewrite all this stuff

// symtable check
bool isDefined(std::string name, int line, int column, std::list<Type> &type) {
        bool defined = true;
        std::optional<Symbol> sym = contextManager.lookup(name);

        if (!sym.has_value()) {
                defined = false;
                std::ostringstream oss;
                oss << "undefined Symbol '" << name << "'";
                oss << " at: " << line << ":" << column << std::endl;
                errMgr.newError(oss.str());
                type = std::list<Type>();
        } else {
                type = sym.value().getType();
        }
        return defined;
}

void printType(std::ostringstream &oss, std::list<Type> types) {
        for (Type t : types) {
                oss << typeToString(t) << " -> ";
        }
        oss << "." << std::endl;
}

void reportDefinitionError(std::string name, int line, int column) {
        std::ostringstream oss;
        oss << "undefined Symbol '" << name << "'";
        oss << " at: " << line << ":" << column << std::endl;
        errMgr.newError(oss.str());
}

void reportTypeError(std::string name, int line, int column,
                     std::list<Type> expected, std::list<Type> founded) {
        std::ostringstream oss;
        oss << "Type error: at " << line << ":" << column << " in " << name
            << ": the expected type was:" << std::endl;
        printType(oss, expected);
        oss << "founded:" << std::endl;
        printType(oss, founded);
        errMgr.newError(oss.str());
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
               Type founded) {
        if (expected != founded && founded != VOID && expected != VOID) {
                std::ostringstream oss;
                oss << "assignment at " << line << ":" << column << " " << name
                    << " is of type " << typeToString(expected)
                    << " but the value assigned is of type "
                    << typeToString(founded) << std::endl;
                errMgr.newWarning(oss.str());
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
