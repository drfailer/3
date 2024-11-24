#ifndef TOOLS_S3C_H
#define TOOLS_S3C_H
#include "ast/ast.hpp"
#include "symtable/context_manager.hpp"
#include "tools/errors_manager.hpp"
#include "tools/program_builder.hpp"

class S3C {
  public:
    ProgramBuilder &programBuilder() { return programBuilder_; }
    Symtable &symtable() { return symtable_; }
    ContextManager &contextManager() { return contextManager_; }
    ErrorManager &errorsManager() { return errorsManager_; }

  public:
    bool newFunctionDefinition(std::string const &functionName, size_t line) {
        programBuilder_.currFunctionName(functionName);
        std::optional<Symbol> sym = contextManager_.lookup(functionName);
        // error on function redefinition
        if (sym.has_value()) {
            errorsManager_.addMultipleDefinitionError(
                programBuilder_.currFileName(), line, functionName);
            // TODO: print the previous definition location
            return false;
        }
        contextManager_.enterScope();
        return true;
    }

    void setFunctionType(type_system::type returnType) {
        type_system::type type = type_system::make_type<type_system::Function>(
            returnType, programBuilder_.parametersTypes());
        contextManager_.newGlobalSymbol(programBuilder_.currFunctionName(),
                                        type, FUNCTION);
    }

    void endFunctionDefintion(std::string const &name,
                              type_system::type returnType,
                              std::shared_ptr<Block> body) {
        programBuilder_.createFunction(name, body, returnType);
        contextManager_.leaveScope();
    }

  public:
    void newParameterDeclaration(std::string const &name,
                                 type_system::type type) {
        contextManager_.newSymbol(name, type, FUN_PARAM);
        programBuilder_.pushFunctionParam(Variable(name, type));
    }

    void newArrayParameterDeclaration(std::string const &name,
                                      type_system::type type, long size) {
        // TODO: remove the size of the array. The size should be set at
        // -1 (or any default value) in order to specify that we don't
        // want to check the size at compile time when we treat the
        // function
        contextManager_.newSymbol(name, type, LOCAL_ARRAY);
        programBuilder_.pushFunctionParam(
            Array(name, size,
                  std::static_pointer_cast<type_system::StaticArray>(type)));
    }

  public:
    void newReturnExpression(std::shared_ptr<TypedNode> expr, size_t line) {
        std::optional<Symbol> sym =
            contextManager_.lookup(programBuilder_.currFunctionName());
        type_system::type foundType = expr->type;
        type_system::type expectedType = sym.value().getType();
        type_system::PrimitiveTypes e_expectedType =
            expectedType->getEvaluatedType();
        type_system::PrimitiveTypes e_foundType = foundType->getEvaluatedType();
        std::ostringstream oss;

        if (e_expectedType == type_system::NIL) { // no return allowed
            errorsManager_.addUnexpectedReturnError(
                programBuilder_.currFileName(), line,
                programBuilder_.currFunctionName());
        } else if (e_expectedType != e_foundType &&
                   e_foundType != type_system::NIL) {
            // TODO: create a function to compare types
            std::cout << "ERROR: type comparison done wrong." << std::endl;
            // must check if foundType is not void because of the
            // buildin function (add, ...) which are not in the
            // symtable
            errorsManager_.addReturnTypeWarning(
                programBuilder_.currFileName(), line,
                programBuilder_.currFunctionName(), foundType, expectedType);
        }
        // else verify the type and throw a warning
        programBuilder_.pushBlock(std::make_shared<Return>(expr));
    }

  public:
    void newShw(std::shared_ptr<TypedNode> expr) {
        if (type_system::isArrayOfChr(expr->type)) {
            auto stringValue = std::dynamic_pointer_cast<Value>(expr);
            std::string str = stringValue->value._str;
            programBuilder_.pushBlock(std::make_shared<Shw>(str));
        } else {
            programBuilder_.pushBlock(std::make_shared<Shw>(expr));
        }
    }

  private:
    ProgramBuilder programBuilder_;
    Symtable symtable_;
    ContextManager contextManager_;
    ErrorManager errorsManager_;
};

#endif
