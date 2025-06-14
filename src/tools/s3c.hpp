#ifndef TOOLS_S3C_H
#define TOOLS_S3C_H
#include "ast/node.hpp"
#include "context_manager.hpp"
#include "symtable/symbol_table.hpp"
#include "symtable/type.hpp"
#include "tools/errors_manager.hpp"
#include "tools/program_builder.hpp"
#include "type_utilities.hpp"
#include <cstring>
#include <functional>
#include <iostream>

#define LOCATION create_location(programBuilder_.currFileName, line)

class S3C {
  public:
    // TODO: create a global state that will replace these classes
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
            auto symbol = contextManager_.lookup(arg_id);

            if (symbol) {
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
    void newReturnExpression(node::Node *expr, size_t line) {
        std::ostringstream oss;
        Symbol *sym = contextManager_.lookup(programBuilder_.currFunctionName);
        auto foundType = get_eval_type(expr, contextManager_.currentScope);
        auto expectedType = sym->type;

        if (foundType.status != eval_type_r::Success) {
            // TODO: print error
            return;
        }

        if (expectedType->kind == type::TypeKind::Nil) { // no return allowed
            errorsManager_.addUnexpectedReturnError(
                programBuilder_.currFileName, line,
                programBuilder_.currFunctionName);
        } else if (type::type_equal(foundType.type, expectedType)) {
            if (type::type_is_convertible(expectedType, foundType.type)) {
                errorsManager_.addReturnTypeWarning(
                    programBuilder_.currFileName, line,
                    programBuilder_.currFunctionName, foundType.type,
                    sym->type);
            } else {
                errorsManager_.addReturnTypeError(
                    programBuilder_.currFileName, line,
                    programBuilder_.currFunctionName, foundType.type,
                    sym->type);
            }
        }
        // else verify the type and throw a warning
        programBuilder_.pushBlock(node::create_ret_stmt(LOCATION, expr));
    }

  public:
    void newShw(node::Node *expr, size_t line) {
        // TODO: check get_eval_type status
        if (expr->kind == node::NodeKind::Value &&
            expr->value.value->kind == node::ValueKind::String) {
            // TODO: create a clean quote function
            programBuilder_.pushBlock(expr);
        } else if (get_eval_type(expr, contextManager_.currentScope)
                       .type->kind == type::TypeKind::Primitive) {
            programBuilder_.pushBlock(node::create_builtin_function(
                LOCATION, node::BuiltinFunctionKind::Shw, expr));
        } else {
            // TODO: this is error is not very clear (note that the syntax is
            // not strong ehough on the shw function)
            errorsManager_.addBadUsageOfShwError(
                programBuilder_.currFileName, line,
                get_eval_type(expr, contextManager_.currentScope).type);
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
            // TODO: check get_eval_type status
            type::Type *index_type =
                get_eval_type(index, contextManager_.currentScope).type;
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
                get_eval_type(lhs, contextManager_.currentScope).type) ||
            !type::is_number(
                get_eval_type(rhs, contextManager_.currentScope).type)) {
            errorsManager_.addOperatorError(programBuilder_.currFileName, line,
                                            operatorName);
        }
        return node::create_arithmetic_operation(LOCATION, kind, lhs, rhs);
    }

  public:
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
                return; // TODO: we should returh a status
            }

            auto arg = functionCall->arguments.begin();
            auto expected_type = function_type->arguments_types.begin();
            for (; expected_type != function_type->arguments_types.end();) {
                if (!type::type_is_convertible(
                        get_eval_type(*arg++, scope).type, *expected_type++)) {
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
            if (!type::type_is_convertible(
                    functionCallSymbol->type->value.function->return_type,
                    variable_symbol->type)) {
                std::cerr << "error: bad assignment TODO: better message"
                          << std::endl;
            }
        } else {
            errorsManager_.addUndefinedSymbolError(fileName, line, expr->name);
        }
    }

    void newAssignment(node::Node *target, node::Node *expr, size_t line) {
        auto icType = get_eval_type(expr, contextManager_.currentScope);
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
        } else if (!type::type_is_convertible(icType.type, symbol->type)) {
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
        auto begin_type = get_eval_type(begin, contextManager_.currentScope);
        auto end_type = get_eval_type(end, contextManager_.currentScope);
        auto step_type = get_eval_type(step, contextManager_.currentScope);
        type::Type *index_type = nullptr;

        if (begin_type.status != eval_type_r::Success ||
            !type::is_number(begin_type.type)) {
            // TODO: print error
            return nullptr;
        }
        index_type = begin_type.type;

        if (end_type.status != eval_type_r::Success ||
            type::is_number(end_type.type)) {
            // TODO: print error
            return nullptr;
        } else if (end_type.type->value.primitive == type::PrimitiveType::Flt) {
            index_type = end_type.type;
        }

        if (step_type.status != eval_type_r::Success ||
            type::is_number(step_type.type)) {
            // TODO: print error
            return nullptr;
        } else if (end_type.type->value.primitive == type::PrimitiveType::Flt) {
            index_type = step_type.type;
        }
        contextManager_.newGlobalSymbol(index_id, index_type);
        contextManager_.leaveScope();
        return node::create_for_stmt(LOCATION, index_id, begin, end, step,
                                     block);
    }

  private:
    ProgramBuilder programBuilder_;
    SymbolTable symtable_;
    ContextManager contextManager_;
    ErrorManager errorsManager_;

    std::list<VerificationFunction> postProcessVerifications_ = {};
};

#endif
