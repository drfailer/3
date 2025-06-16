#include "s3c.hpp"
#include "symbol_table.hpp"

namespace s3c {

State *state_create() {
    auto *state = new State();
    auto *scope = symbol_table_create(nullptr);
    state->scopes.global = scope;
    state->scopes.curr = scope;
    return state;
}

void post_process(State *state) {
    for (auto callback : state->post_process_callbacks) {
        callback();
    }
}

void reset_curr_function(State *state) {
    state->curr_function.name.clear();
    state->curr_function.arguments.clear();
    state->curr_function.arguments_types.clear();
    state->curr_function.return_type = nullptr;
    state->curr_function.blocks = {};
}

void enter_file(State *state, std::string const &filename) {
    if (filename.empty()) {
        return;
    }

    state->curr_filename = filename[0] == '"' ? filename.substr(1) : filename;
    if (filename.back() == '"') {
        state->curr_filename.pop_back();
    }
}

void enter_function(State *state, std::string const &function_name) {
    state->curr_function.name = function_name;
}

void enter_scope(State *state) {
    auto scope = symbol_table_create(state->scopes.curr);
    state->scopes.curr->childs_scopes.push_back(scope);
    state->scopes.curr = scope;
}

void leave_scope(State *state) {
    state->scopes.curr = state->scopes.curr->parent;
}

void add_symbol(State *state, std::string const &id, type::Type *type) {
    insert_symbol(state->scopes.curr, id, type);
}

void add_global_symbol(State *state, std::string const &id, type::Type *type) {
    insert_symbol(state->scopes.global, id, type);
}

node::Node *new_argument_declaration(State *state, std::string const id,
                                     type::Type *type, size_t line) {
    auto node = node::create_variable_reference(LOCATION, id);
    add_symbol(state, id, type);
    state->curr_function.arguments.push_back(node);
    state->curr_function.arguments_types.push_back(type);
    return node;
}

int new_function_definition(State *state, std::string const &id, size_t line) {
    state->curr_function.name = id;
    Symbol *sym = lookup(state->scopes.global, id);
    // error on function redefinition
    if (sym) {
        // TODO: print the previous definition location
        // TODO: print error message
        // errorsManager_.addMultipleDefinitionError(
        // programBuilder_.currFileName, line, functionName);
        return 1;
    }
    enter_scope(state);
    return 0;
}

void set_curr_function_type(State *state, type::Type *return_type) {
    add_global_symbol(state, state->curr_function.name,
                      type::create_function_type(
                          state->curr_function.name, return_type,
                          std::move(state->curr_function.arguments_types)));
}

void add_function_definition(State *state, std::string const &name,
                             node::Block *body, size_t line) {
    state->program.push_back(node::create_function_definition(
        LOCATION, name, state->curr_function.arguments, body));
    leave_scope(state);
    reset_curr_function(state);
}

void begin_block(State *state) {
    state->curr_function.blocks.push(new node::Block());
}

node::Block *end_block(State *state) {
    node::Block *lastBlock = state->curr_function.blocks.top();
    state->curr_function.blocks.pop();
    return lastBlock;
}

void add_instruction(State *state, node::Node *node) {
    state->curr_function.blocks.top()->nodes.push_back(node);
}

void new_return_expr(State *state, node::Node *expr, size_t line) {
    std::ostringstream oss;
    Symbol *sym = lookup(state->scopes.curr, state->curr_function.name);
    auto foundType = get_eval_type(expr, state->scopes.curr);
    auto expectedType = sym->type;

    if (foundType.status != eval_type_r::Success) {
        // TODO: print error
        return;
    }

    if (expectedType->kind == type::TypeKind::Nil) { // no return allowed
        // TODO
        // errorsManager_.addUnexpectedReturnError(
        //     programBuilder_.currFileName, line,
        //     programBuilder_.currFunctionName);
    } else if (type::type_equal(foundType.type, expectedType)) {
        if (type::type_is_convertible(expectedType, foundType.type)) {
            // TODO
            // errorsManager_.addReturnTypeWarning(
            //     programBuilder_.currFileName, line,
            //     programBuilder_.currFunctionName, foundType.type,
            //     sym->type);
        } else {
            // TODO
            // errorsManager_.addReturnTypeError(
            //     programBuilder_.currFileName, line,
            //     programBuilder_.currFunctionName, foundType.type,
            //     sym->type);
        }
    }
    // else verify the type and throw a warning
    add_instruction(state, node::create_ret_stmt(LOCATION, expr));
}

node::Node *new_arithmetic_operation(State *state, node::Node *lhs,
                                     node::Node *rhs,
                                     node::ArithmeticOperationKind kind,
                                     size_t line,
                                     std::string const &operatorName) {
    // TODO: there are plenty of cases that are not handled here
    // TODO: if one of the operands is an undefined function (None type),
    // make the verification in post process
    if (!type::is_number(get_eval_type(lhs, state->scopes.curr).type) ||
        !type::is_number(get_eval_type(rhs, state->scopes.curr).type)) {
        // TODO
        // errorsManager_.addOperatorError(programBuilder_.currFileName, line,
        //                                 operatorName);
    }
    return node::create_arithmetic_operation(LOCATION, kind, lhs, rhs);
}

void very_function_call_type(std::string const &functionName,
                             node::FunctionCall *functionCall,
                             SymbolTable *scope, std::string const &fileName,
                             size_t line) {
    auto sym = lookup(scope, functionName);

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
            if (!type::type_is_convertible(get_eval_type(*arg++, scope).type,
                                           *expected_type++)) {
                // TODO: improve this error message
                std::cerr << "error: no matching function call to "
                          << functionName << "." << std::endl;
            }
        }
    } else {
        // TODO
        // errorsManager_.addUndefinedSymbolError(fileName, line, functionName);
    }
}

void save_function_call_argument(State *state, node::Node *node) {
    state->funcall_parameters.back().push_back(node);
}

void begin_new_funcall(State *state, std::string const &id) {
    state->funcall_ids.push_back(id);
    state->funcall_parameters.push_back({});
}

node::Node *new_function_call(State *state, std::string const &id,
                              size_t line) {
    // TODO: we need a none type as if the function is not defined, we
    // cannot get the return type (should be verified afterward)
    auto args = state->funcall_parameters.back();
    auto *funcall = node::create_function_call({}, state->funcall_ids.back(),
                                               std::move(args));
    state->funcall_ids.pop_back();
    state->funcall_parameters.pop_back();

    // setup type verification
    std::string file = state->curr_filename;
    auto scope = state->scopes.curr;
    state->post_process_callbacks.push_back([=]() {
        very_function_call_type(id, funcall->value.function_call, scope, file,
                                line);
    });
    return funcall;
}

void new_variable_declaration(State *state, std::string id, type::Type *type,
                              size_t line) {
    auto symbol = state->scopes.curr->symbols.find(id);

    // redefinitions are not allowed:
    if (symbol != state->scopes.curr->symbols.end()) {
        // TODO
        // errorsManager_.addMultipleDefinitionError(programBuilder_.currFileName,
        //                                           line, variableName);
    }
    insert_symbol(state->scopes.curr, id, type);
    add_instruction(state, node::create_variable_definition(LOCATION, id));
}

node::Node *new_variable(State *state, std::string const &name, size_t line) {
    auto sym = lookup(state->scopes.curr, name);

    if (!sym) {
        // TODO
        // errorsManager_.addUndefinedSymbolError(programBuilder_.currFileName,
        //                                        line, name);
    }
    return node::create_variable_reference(LOCATION, name);
}

node::Node *new_index_expr(State *state, std::string const &name, size_t line,
                           node::Node *index_node) {
    auto variable = node::create_variable_reference(LOCATION, name);
    auto index_expr =
        node::create_index_expression(LOCATION, variable, index_node);
    auto sym = lookup(state->scopes.curr, name);

    if (!sym) {
        // TODO
        // errorsManager_.addUndefinedSymbolError(programBuilder_.currFileName,
        //                                        line, name);
    } else if (sym->type->kind != type::TypeKind::Array) {
        // TODO
        // errorsManager_.addBadArrayUsageError(programBuilder_.currFileName,
        //                                      line, name);
    } else {
        // TODO: check get_eval_type status
        type::Type *index_type =
            get_eval_type(index_node, state->scopes.curr).type;
        bool index_is_int =
            index_type->kind == type::TypeKind::Primitive &&
            index_type->value.primitive == type::PrimitiveType::Int;
        if (!index_is_int) {
            // TODO: error: invalid index type
        }
    }
    return index_expr;
}

void very_assignement_type(State *state, node::Assignment *assignment,
                           node::FunctionCall *expr, std::string const &file,
                           size_t line) {
    Symbol *variable_symbol = nullptr;

    if (assignment->target->kind == node::NodeKind::VariableReference) {
        variable_symbol =
            lookup(state->scopes.curr,
                   assignment->target->value.variable_reference->name);
    } else if (assignment->target->kind == node::NodeKind::IndexExpression) {
        variable_symbol = lookup(state->scopes.curr,
                                 get_lvalue_identifier(assignment->target));
        // TODO: the error message inside get_indexed_identifier should here
    } else {
        std::cerr << "error: try to assign invalid expression." << std::endl;
    }

    if (auto functionCallSymbol = lookup(state->scopes.curr, expr->name)) {
        if (!type::type_is_convertible(
                functionCallSymbol->type->value.function->return_type,
                variable_symbol->type)) {
            std::cerr << "error: bad assignment TODO: better message"
                      << std::endl;
        }
    } else {
        // TODO
        // errorsManager_.addUndefinedSymbolError(file, line, expr->name);
    }
}

void new_assignment(State *state, node::Node *target, node::Node *expr,
                    size_t line) {
    auto icType = get_eval_type(expr, state->scopes.curr);
    auto node = node::create_assignment(LOCATION, target, expr);
    auto variable_name = get_lvalue_identifier(target);
    auto symbol = lookup(state->scopes.curr, variable_name);

    if (!symbol) {
        // TODO
        std::cerr << "error: use of undefined variable " << variable_name
                  << std::endl;
        return;
    }

    if (expr->kind == node::NodeKind::FunctionCall) {
        // this is a funcall so we have to wait the end of the parsing to
        // check (this is due to the fact that a function can be used before
        // it's defined).
        state->post_process_callbacks.push_back([=]() {
            very_assignement_type(state, node->value.assignment,
                                  expr->value.function_call,
                                  state->curr_filename, line);
        });
    } else if (!type::type_is_convertible(icType.type, symbol->type)) {
        std::cerr << "error: invalid assignment" << std::endl;
    }
    add_instruction(state, node);
    // TODO: check the type for strings -> array of char
}

node::Node *new_for(State *state, std::string const &index_id,
                    node::Node *begin, node::Node *end, node::Node *step,
                    node::Block *block, size_t line) {
    std::cerr << "TODO: verify rng types" << std::endl;
    auto begin_type = get_eval_type(begin, state->scopes.curr);
    auto end_type = get_eval_type(end, state->scopes.curr);
    auto step_type = get_eval_type(step, state->scopes.curr);
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
    add_global_symbol(state, index_id, index_type);
    leave_scope(state);
    return node::create_for_stmt(
        LOCATION, node::create_variable_definition(LOCATION, index_id), begin,
        end, step, block);
}

void new_shw(State *state, node::Node *expr, size_t line) {
    // TODO: check get_eval_type status
    if (expr->kind == node::NodeKind::Value &&
        expr->value.value->kind == node::ValueKind::String) {
        // TODO: create a clean quote function
        add_instruction(state, expr);
    } else if (get_eval_type(expr, state->scopes.curr).type->kind ==
               type::TypeKind::Primitive) {
        add_instruction(state,
                        node::create_builtin_function(
                            LOCATION, node::BuiltinFunctionKind::Shw, expr));
    } else {
        // TODO: this is error is not very clear (note that the syntax is
        // not strong ehough on the shw function)
        // TODO
        // errorsManager_.addBadUsageOfShwError(
        //     programBuilder_.currFileName, line,
        //     get_eval_type(expr, contextManager_.currentScope).type);
    }
}

} // end namespace s3c
