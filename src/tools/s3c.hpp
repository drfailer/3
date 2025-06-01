#ifndef TOOLS_S3C_H
#define TOOLS_S3C_H
#include "ast/node.hpp"
#include "context_manager.hpp"
#include "symtable/symbol_table.hpp"
#include "symtable/type.hpp"
#include "tools/errors_manager.hpp"
#include "tools/program_builder.hpp"
#include <cstring>
#include <functional>
#include <iostream>

#define LOCATION create_location(programBuilder_.currFileName, line)

class S3C {
  public:
    ProgramBuilder &programBuilder() { return programBuilder_; }
    SymbolTable &symtable() { return symtable_; }
    ContextManager &contextManager() { return contextManager_; }
    ErrorManager &errorsManager() { return errorsManager_; }

    using VerificationFunction = std::function<void(void)>;

  public:
    // Move outside the class ?
    void runPostProcessVerifications() {
        for (auto verify : postProcessVerifications_) {
            verify();
        }
    }

  public:
    bool newFunctionDefinition(std::string const &functionName, size_t line) {
        programBuilder_.currFunctionName = functionName;
        Symbol *sym = contextManager_.lookup(functionName);
        // error on function redefinition
        if (sym) {
            errorsManager_.addMultipleDefinitionError(
                programBuilder_.currFileName, line, functionName);
            // TODO: print the previous definition location
            return false;
        }
        contextManager_.enterScope();
        return true;
    }

    void setFunctionType(type::Type *returnType) {
        std::list<type::Type *> arguments_types;

        for (auto arg_id : programBuilder_.currFunctionParameters) {
            std::cout << arg_id << std::endl;
            auto symbol = contextManager_.lookup(arg_id);

            if (symbol) {
                std::cout << type::type_to_string(symbol->type) << std::endl;
                arguments_types.push_back(symbol->type);
            } else {
                // this should never happen
                std::cerr << "error: the function argument is not in the table "
                             "of symbol"
                          << std::endl;
            }
        }
        contextManager_.newGlobalSymbol(
            programBuilder_.currFunctionName,
            type::create_function_type(programBuilder_.currFunctionName,
                                       returnType, std::move(arguments_types)));
    }

    void endFunctionDefintion(std::string const &name, node::Block *body) {
        programBuilder_.createFunction(name, body);
        contextManager_.leaveScope();
    }

  public:
    void newParameterDeclaration(std::string const &id, type::Type *type) {
        contextManager_.newSymbol(id, type);
        programBuilder_.pushFunctionParam(id);
    }

  public:
    // TODO: we need a function that finds the type of an node
    void newReturnExpression(node::Node *expr, size_t line) {
        std::ostringstream oss;
        Symbol *sym = contextManager_.lookup(programBuilder_.currFunctionName);
        auto foundType = get_evaluated_type(expr, contextManager_.currentScope);
        auto expectedType = sym->type;

        if (foundType->kind == type::TypeKind::Nil) {
            std::cerr << "ERROR: none type found" << std::endl;
            throw std::runtime_error("error: not implemented");
        }

        if (expectedType->kind == type::TypeKind::Nil) { // no return allowed
            errorsManager_.addUnexpectedReturnError(
                programBuilder_.currFileName, line,
                programBuilder_.currFunctionName);
        } else if (type::type_equal(foundType, expectedType)) {
            if (type::type_is_convertible(expectedType, foundType)) {
                errorsManager_.addReturnTypeWarning(
                    programBuilder_.currFileName, line,
                    programBuilder_.currFunctionName, foundType, sym->type);
            } else {
                errorsManager_.addReturnTypeError(
                    programBuilder_.currFileName, line,
                    programBuilder_.currFunctionName, foundType, sym->type);
            }
        }
        // else verify the type and throw a warning
        programBuilder_.pushBlock(node::create_ret_stmt(LOCATION, expr));
    }

  public:
    void newShw(node::Node *expr, size_t line) {
        if (expr->kind == node::NodeKind::Value &&
            expr->value.value->kind == node::ValueKind::String) {
            // TODO: create a clean quote function
            programBuilder_.pushBlock(expr);
        } else if (get_evaluated_type(expr, contextManager_.currentScope)
                       ->kind == type::TypeKind::Primitive) {
            programBuilder_.pushBlock(node::create_builtin_function(
                LOCATION, node::BuiltinFunctionKind::Shw, expr));
        } else {
            // TODO: this is error is not very clear (note that the syntax is
            // not strong ehough on the shw function)
            errorsManager_.addBadUsageOfShwError(
                programBuilder_.currFileName, line,
                get_evaluated_type(expr, contextManager_.currentScope));
        }
    }

  public:
    node::Node *newVariable(std::string const &name, size_t line) {
        auto sym = contextManager_.lookup(name);

        if (!sym) {
            errorsManager_.addUndefinedSymbolError(programBuilder_.currFileName,
                                                   line, name);
        }
        return node::create_variable_reference(LOCATION, name);
    }

    node::Node *newArrayVariable(std::string const &name, size_t line,
                                 node::Node *index) {
        auto variable = node::create_variable_reference(LOCATION, name);
        auto index_expr =
            node::create_index_expression(LOCATION, variable, index);
        auto sym = contextManager_.lookup(name);

        if (!sym) {
            errorsManager_.addUndefinedSymbolError(programBuilder_.currFileName,
                                                   line, name);
        } else if (sym->type->kind != type::TypeKind::Array) {
            errorsManager_.addBadArrayUsageError(programBuilder_.currFileName,
                                                 line, name);
        } else {
            type::Type *index_type =
                get_evaluated_type(index, contextManager_.currentScope);
            bool index_is_int =
                index_type->kind == type::TypeKind::Primitive &&
                index_type->value.primitive == type::PrimitiveType::Int;
            if (!index_is_int) {
                // TODO: error: invalid index type
            }
        }
        return index_expr;
    }

  public:
    node::Node *newArithmeticOperator(node::Node *lhs, node::Node *rhs,
                                      node::ArithmeticOperationKind kind,
                                      size_t line,
                                      std::string const &operatorName) {
        // TODO: if one of the operands is an undefined function (None type),
        // make the verification in post process
        if (!type::is_number(
                get_evaluated_type(lhs, contextManager_.currentScope)) ||
            !type::is_number(
                get_evaluated_type(rhs, contextManager_.currentScope))) {
            errorsManager_.addOperatorError(programBuilder_.currFileName, line,
                                            operatorName);
        }
        return node::create_arithmetic_operation(LOCATION, kind, lhs, rhs);
    }

  public:
    // TODO: move out of the class
    // type_system::types_t
    // getFunctionCallParametersTypes(std::list<node::TypedNode> *const &nodes)
    // {
    //     type_system::types_t parametersTypes = {};
    //
    //     std::transform(
    //         nodes.cbegin(), nodes.cend(),
    //         std::back_insert_iterator<type_system::types_t>(parametersTypes),
    //         [](auto elt) { return elt->type; });
    //     return parametersTypes;
    // }

    // TODO: refactor this function
    void verifyFunctionCallType(std::string const &functionName,
                                node::FunctionCall *functionCall,
                                SymbolTable *scope, std::string const &fileName,
                                size_t line) {
        contextManager_.currentScope = scope;
        auto sym = contextManager_.lookup(functionName);

        if (sym) {
            if (sym->type->kind != type::TypeKind::Function) {
                std::cerr << "error: " << sym->id << " used as a function."
                          << std::endl;
            }
            auto function_type = sym->type->value.function;

            if (function_type->arguments_types.size() !=
                functionCall->arguments.size()) {
                std::cerr << "error: wrong number of arguments for '"
                          << functionName << "'." << std::endl;
                std::cerr << "expecte type: " << type::type_to_string(sym->type)
                          << std::endl;
                std::cerr << "found " << std::endl;
                for (auto arg : functionCall->arguments) {
                    std::cout << "arg node kind: " << (int)arg->kind
                              << std::endl;
                }
                return; // TODO: we should returh a status
            }

            auto arg = functionCall->arguments.begin();
            auto expected_type = function_type->arguments_types.begin();
            for (; expected_type != function_type->arguments_types.end();) {
                if (!type::type_is_convertible(get_evaluated_type(*arg++, scope),
                                              *expected_type++)) {
                    // TODO: improve this error message
                    std::cerr << "error: no matching function call to "
                              << functionName << "." << std::endl;
                }
            }
        } else {
            errorsManager_.addUndefinedSymbolError(fileName, line,
                                                   functionName);
        }
    }

    node::Node *newFunctionCall(std::string const &functionName, size_t line) {
        // TODO: we need a none type as if the function is not defined, we
        // cannot get the return type (should be verified afterward)
        auto *funcall = programBuilder_.createFuncall();

        // setup type verification
        std::string file = programBuilder_.currFileName;
        auto scope = contextManager_.currentScope;
        postProcessVerifications_.push_back([=]() {
            verifyFunctionCallType(functionName, funcall->value.function_call,
                                   scope, file, line);
        });
        return funcall;
    }

  public:
    void newVariableDeclaration(std::string variableName, type::Type *type,
                                size_t line) {
        auto symbol = contextManager_.currentScope->symbols.find(variableName);

        // redefinitions are not allowed:
        if (symbol != contextManager_.currentScope->symbols.end()) {
            errorsManager_.addMultipleDefinitionError(
                programBuilder_.currFileName, line, variableName);
        }
        contextManager_.newSymbol(variableName, type);
        programBuilder_.pushBlock(
            node::create_variable_definition(LOCATION, variableName));
    }

  public:
    void verifyAssignmentType(node::Assignment *assignment,
                              node::FunctionCall *expr,
                              std::string const &fileName, size_t line) {
        Symbol *variable_symbol = nullptr;

        if (assignment->target->kind == node::NodeKind::VariableReference) {
            variable_symbol = contextManager_.lookup(
                assignment->target->value.variable_reference->name);
        } else if (assignment->target->kind ==
                   node::NodeKind::IndexExpression) {
            variable_symbol = contextManager_.lookup(
                get_lvalue_identifier(assignment->target));
            // TODO: the error message inside get_indexed_identifier should here
        } else {
            std::cerr << "error: try to assign invalid expression."
                      << std::endl;
        }

        if (auto functionCallSymbol = contextManager_.lookup(expr->name)) {
            if (!type::type_is_convertible(functionCallSymbol->type,
                                           variable_symbol->type)) {
                std::cerr << "error: bad assignment TODO: better message"
                          << std::endl;
            }
        } else {
            errorsManager_.addUndefinedSymbolError(fileName, line, expr->name);
        }
    }

    void newAssignment(node::Node *target, node::Node *expr, size_t line) {
        auto icType = get_evaluated_type(expr, contextManager_.currentScope);
        auto newAssignment = node::create_assignment(LOCATION, target, expr);
        auto variable_name = get_lvalue_identifier(target);
        auto symbol = contextManager_.lookup(variable_name);

        if (!symbol) {
            std::cerr << "error: use of undefined variable " << variable_name
                      << std::endl;
            return;
        }

        if (expr->kind == node::NodeKind::FunctionCall) {
            // this is a funcall so we have to wait the end of the parsing to
            // check (this is due to the fact that a function can be used before
            // it's defined).
            postProcessVerifications_.push_back([=]() {
                verifyAssignmentType(newAssignment->value.assignment,
                                     expr->value.function_call,
                                     programBuilder_.currFileName, line);
            });
        } else if (!type::type_is_convertible(icType, symbol->type)) {
            std::cerr << "error: invalid assignment" << std::endl;
        }
        programBuilder_.pushBlock(newAssignment);
        // TODO: check the type for strings -> array of char
    }

  public:
    node::Node *newFor(std::string const &index_id, node::Node *begin,
                       node::Node *end, node::Node *step, node::Block *block,
                       size_t line) {
        std::cerr << "TODO: verify rng types" << std::endl;
        // Variable variable(
        //     variableName,
        //     type_system::make_type<type_system::Primitive>(type_system::NIL));
        // type::Type *type;
        //
        // if (isDefined(*this, programBuilder_.currFileName(), line,
        // variableName,
        //               type)) {
        //     variable.type = type;
        //     checkType(*this, programBuilder_.currFileName(), line,
        //               "RANGE_BEGIN", type, begin->type->getEvaluatedType());
        //     checkType(*this, programBuilder_.currFileName(), line,
        //     "RANGE_END",
        //               type, end->type->getEvaluatedType());
        //     checkType(*this, programBuilder_.currFileName(), line,
        //     "RANGE_STEP",
        //               type, step->type->getEvaluatedType());
        // }
        contextManager_.leaveScope();
        return node::create_for_stmt(LOCATION, index_id, begin, end, step,
                                     block);
    }

    type::Type *get_evaluated_type(node::Node *node, SymbolTable *scope) {
        std::cout << (int)node->kind << std::endl;
        switch (node->kind) {
        case node::NodeKind::Value:
            switch (node->value.value->kind) {
            case node::ValueKind::Character:
                return type::create_primitive_type(type::PrimitiveType::Chr);
            case node::ValueKind::Integer:
                return type::create_primitive_type(type::PrimitiveType::Int);
            case node::ValueKind::Real:
                return type::create_primitive_type(type::PrimitiveType::Flt);
            case node::ValueKind::String:
                return type::create_primitive_type(type::PrimitiveType::Str);
            }
            break;
        case node::NodeKind::VariableReference: {
            auto id = get_lvalue_identifier(node);
            auto symbol = lookup(scope, id);
            return symbol->type;
        } break;
        case node::NodeKind::IndexExpression: {
            return get_evaluated_type(node->value.index_expression->element,
                                      contextManager_.currentScope);
        } break;
        case node::NodeKind::ArithmeticOperation: {
            auto lhs_type =
                get_evaluated_type(node->value.arithmetic_operation->lhs,
                                   contextManager_.currentScope);
            auto rhs_type =
                get_evaluated_type(node->value.arithmetic_operation->rhs,
                                   contextManager_.currentScope);

            std::cout << type::type_to_string(lhs_type) << std::endl;
            std::cout << type::type_to_string(rhs_type) << std::endl;

            if (lhs_type->kind != type::TypeKind::Primitive ||
                rhs_type->kind != type::TypeKind::Primitive) {
                std::cerr << "error: cannot use arithmetic operator on a non "
                             "primitive type."
                          << std::endl;
                return nullptr;
            }
            // Flt > Int so if one operand is of type Flt, the evaluated type is
            // Flt and not Int.
            if (lhs_type->value.primitive == type::PrimitiveType::Flt) {
                return lhs_type;
            }
            return rhs_type;
        } break;
        case node::NodeKind::BooleanOperation:
            // TODO: write dedicated function for boolean operations
            std::cerr
                << "TODO: IMPLEMENT GET_EVALUATED_TYPE for BOOLEANOPERATION"
                << std::endl;
            break;
        case node::NodeKind::FunctionCall: {
            auto id = node->value.function_call->name;
            auto symbol = lookup(scope, id);
            return symbol->type->value.function->return_type;
        } break;
        default:
            std::cerr << "error: the given node is not evaluable." << std::endl;
            break;
        }
        return type::create_nil_type();
    }

    std::string get_lvalue_identifier(node::Node *target) {
        if (target->kind == node::NodeKind::VariableReference) {
            return target->value.variable_reference->name;
        } else if (target->kind == node::NodeKind::IndexExpression) {
            return get_lvalue_identifier(target->value.index_expression->index);
        } else {
            std::cerr << "error: the target node is not an lvalue" << std::endl;
        }
        return "";
    }

  private:
    ProgramBuilder programBuilder_;
    SymbolTable symtable_;
    ContextManager contextManager_;
    ErrorManager errorsManager_;

    std::list<VerificationFunction> postProcessVerifications_ = {};
};

#endif
