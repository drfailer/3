#ifndef TOOLS_S3C_H
#define TOOLS_S3C_H
#include "ast/ast.hpp"
#include "symtable/context_manager.hpp"
#include "tools/checks.hpp"
#include "tools/errors_manager.hpp"
#include "tools/program_builder.hpp"
#include <cstring>

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

  public:
    std::shared_ptr<Variable> newVariable(std::string const &name,
                                          size_t line) {
        type_system::type type;
        std::shared_ptr<Variable> v;

        // TODO: this is really bad, the function isDefined will be changed !
        if (isDefined(*this, programBuilder_.currFileName(), line, name,
                      type)) {
            if (isArray(type)) {
                Symbol sym = contextManager_.lookup(name).value();
                v = std::make_shared<Array>(name, getArraySize(sym.getType()),
                                            type);
            } else {
                v = std::make_shared<Variable>(name, type);
            }
        } else {
            v = std::make_shared<Variable>(
                name, type_system::make_type<type_system::Primitive>(
                          type_system::NIL));
        }
        return v;
    }

    std::shared_ptr<ArrayAccess>
    newArrayVariable(std::string const &name, size_t line,
                     std::shared_ptr<TypedNode> index) {
        type_system::type type;
        std::shared_ptr<ArrayAccess> v;

        // TODO: refactor isDefined
        if (isDefined(*this, programBuilder_.currFileName(), line, name,
                      type)) {
            std::optional<Symbol> sym = contextManager_.lookup(name);
            // error if the symbol is not an array
            if (sym.value().getKind() != LOCAL_ARRAY) {
                errorsManager_.addBadArrayUsageError(
                    programBuilder_.currFileName(), line, name);
            }
            v = std::make_shared<ArrayAccess>(name, type, index);
        } else {
            // TODO: verify the type of the index
            v = std::make_shared<ArrayAccess>(
                name,
                type_system::make_type<type_system::Primitive>(
                    type_system::NIL),
                index);
        }
        return v;
    }

  public:
    template <typename OperatorType>
    std::shared_ptr<TypedNode>
    newArithmeticOperator(std::shared_ptr<TypedNode> lhs,
                          std::shared_ptr<TypedNode> rhs, size_t line,
                          std::string const &operatorName) {
        if (!isNumber(lhs->type) || !isNumber(rhs->type)) {
            errorsManager_.addOperatorError(programBuilder_.currFileName(),
                                            line, operatorName);
        }
        return std::make_shared<OperatorType>(lhs, rhs);
    }

  public:
    template <typename T>
    std::shared_ptr<TypedNode> newFunctionCall(std::string const &functionName,
                                               size_t line,
                                               T &funcallsToCheck) {
        // TODO: we need a none type as if the function is not defined, we
        // cannot get the return type (should be verified afterward)
        std::shared_ptr<FunctionCall> funcall;
        if (std::optional<Symbol> symbol =
                contextManager_.lookup(functionName)) {
            funcall = programBuilder_.createFuncall(
                type_system::make_type<type_system::Primitive>(
                    symbol.value().getType()->getEvaluatedType()));
        } else {
            funcall = programBuilder_.createFuncall();
        }
        std::pair<std::string, int> position =
            std::make_pair(programBuilder_.currFileName(), line);
        // the type check is done at the end !
        funcallsToCheck.push_back(std::make_pair(funcall, position));
        // check the type
        return funcall;
    }

  public:
    void newVariableDeclaration(std::string variableName,
                                type_system::type type, size_t line) {
        // redefinitions are not allowed:
        if (std::optional<Symbol> symbol =
                contextManager_.lookup(variableName)) {
            errorsManager_.addMultipleDefinitionError(
                programBuilder_.currFileName(), line, variableName);
        }
        contextManager_.newSymbol(variableName, type, LOCAL_VAR);
        programBuilder_.pushBlock(
            std::make_shared<Declaration>(Variable(variableName, type)));
    }

    void newArrayDeclaration(std::string variableName, type_system::type type,
                             size_t size, size_t line) {
        // redefinitions are not allowed:
        if (std::optional<Symbol> symbol =
                contextManager_.lookup(variableName)) {
            errorsManager_.addMultipleDefinitionError(
                programBuilder_.currFileName(), line, variableName);
        }
        contextManager_.newSymbol(variableName, type, size, LOCAL_ARRAY);
        programBuilder_.pushBlock(
            std::make_shared<ArrayDeclaration>(variableName, size, type));
    }

  public:
    void newAssignment(
        std::shared_ptr<Variable> variable, std::shared_ptr<TypedNode> expr,
        size_t line,
        std::list<std::pair<std::shared_ptr<Assignment>,
                            std::pair<std::string, int>>> &assignmentsToCheck) {
        auto icType = expr->type;
        auto newAssignment = std::make_shared<Assignment>(variable, expr);

        if (std::static_pointer_cast<FunctionCall>(expr)) { // if funcall
            // this is a funcall so we have to wait the end of the parsing to
            // check
            auto position =
                std::make_pair(programBuilder_.currFileName(), line);
            assignmentsToCheck.push_back(std::pair(newAssignment, position));
        } else {
            checkType(*this, programBuilder_.currFileName(), line, variable->id,
                      variable->type, icType->getEvaluatedType());
        }
        programBuilder_.pushBlock(newAssignment);
        // TODO: check the type for strings -> array of char
    }

  public:
    std::shared_ptr<Value> newInt(long long intValue) {
        type_system::LiteralValue value = {._int = intValue};
        return std::make_shared<Value>(
            value,
            type_system::make_type<type_system::Primitive>(type_system::INT));
    }

    std::shared_ptr<Value> newFlt(double fltValue) {
        type_system::LiteralValue value = {._flt = fltValue};
        return std::make_shared<Value>(
            value,
            type_system::make_type<type_system::Primitive>(type_system::FLT));
    }

    std::shared_ptr<Value> newChr(char chrValue) {
        type_system::LiteralValue value = {._chr = chrValue};
        return std::make_shared<Value>(
            value,
            type_system::make_type<type_system::Primitive>(type_system::CHR));
    }

    std::shared_ptr<Value> newStr(std::string const &strValue, size_t line) {
        type_system::LiteralValue value = {0};
        if (strValue.size() > MAX_LITERAL_STRING_LENGTH) {
            errorsManager_.addLiteralStringOverflowError(
                programBuilder_.currFileName(), line);
            return nullptr;
        }
        memcpy(value._str, strValue.c_str(), strValue.size());
        return std::make_shared<Value>(
            value, type_system::make_type<type_system::StaticArray>(
                       type_system::CHR, strValue.size()));
    }

  public:
    std::shared_ptr<For> newFor(std::string const &variableName,
            std::shared_ptr<TypedNode> begin, std::shared_ptr<TypedNode> end,
            std::shared_ptr<TypedNode> step, std::shared_ptr<Block> block, size_t line) {
        Variable variable(variableName, type_system::make_type<type_system::Primitive>(type_system::NIL));
        type_system::type type;

        if (isDefined(*this, programBuilder_.currFileName(), line, variableName, type)) {
            variable.type = type;
            checkType(*this, programBuilder_.currFileName(), line, "RANGE_BEGIN", type, begin->type->getEvaluatedType());
            checkType(*this, programBuilder_.currFileName(), line, "RANGE_END",  type, end->type->getEvaluatedType());
            checkType(*this, programBuilder_.currFileName(), line, "RANGE_STEP", type, step->type->getEvaluatedType());
        }
        contextManager_.leaveScope();
        return programBuilder_.createFor(variable, begin, end, step, block);
    }

  private:
    ProgramBuilder programBuilder_;
    Symtable symtable_;
    ContextManager contextManager_;
    ErrorManager errorsManager_;
};

#endif
