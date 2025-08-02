#include "s3c.hpp"
#include "symbol_table.hpp"
#include "tools/messages.hpp"
#include "tools/type_utilities.hpp"
#include "tree/location.hpp"
#include "tree/node.hpp"
#include "type/predicates.hpp"
#include "type/type.hpp"
#include <sstream>

// TODO: add an error status in the state

namespace s3c {

State *state_create() {
    auto *state = new State();
    auto *scope = symbol_table_create(nullptr);
    state->scopes.global = scope;
    state->scopes.curr = scope;
    state->status = 0;
    return state;
}

bool post_process(State *state) {
    for (auto callback : state->post_process_callbacks) {
        if (!callback()) {
            return false;
        }
    }
    return true;
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

void leave_scope(State *state, node::Block *block) {
    auto scope = state->scopes.curr;
    state->scopes.curr = state->scopes.curr->parent;
    if (block != nullptr) {
        state->scopes.curr->block_scopes[block] = scope;
    }
}

void add_symbol(State *state, std::string const &id, type::Type *type,
                Location const &location) {
    insert_symbol(state->scopes.curr, id, type, nullptr, location);
}

void add_global_symbol(State *state, std::string const &id, type::Type *type,
                       SymbolTable *scope, Location const &location) {
    insert_symbol(state->scopes.global, id, type, scope, location);
}

node::Node *new_argument_declaration(State *state, std::string const &id,
                                     type::Type *type, size_t line) {
    auto location = location_create(state->curr_filename, line);
    auto node = node::create_variable_definition(location, id);
    add_symbol(state, id, type, location);
    state->curr_function.arguments.push_back(node);
    state->curr_function.arguments_types.push_back(type);
    return node;
}

void new_function_definition(State *state, std::string const &id, size_t line) {
    state->curr_function.name = id;
    Symbol *sym = lookup_id(state->scopes.global, id);

    if (sym) {
        MULTIPLE_DEFINITION_ERROR(LOCATION, id, sym->location);
        state->status = 1;
    }
    enter_scope(state);
}

void set_curr_function_type(State *state, type::Type *return_type,
                            size_t line) {
    add_global_symbol(state, state->curr_function.name,
                      type::create_function_type(
                          state->curr_function.name, return_type,
                          std::move(state->curr_function.arguments_types)),
                      state->scopes.curr, LOCATION);
}

void add_function_definition(State *state, std::string const &name,
                             node::Block *body, size_t line) {
    state->program.push_back(node::create_function_definition(
        LOCATION, name, state->curr_function.arguments, body));
    leave_scope(state, body);
    reset_curr_function(state);
}

void add_function_declaration(State *state, size_t line) {
    state->program.push_back(node::create_function_declaration(
        LOCATION, state->curr_function.name, state->curr_function.arguments));
    s3c::leave_scope(state, nullptr);
    s3c::reset_curr_function(state);
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
    Symbol *sym = lookup_id(state->scopes.curr, state->curr_function.name);
    auto expected_type = sym->type->value.function->return_type;
    auto node = node::create_ret_stmt(LOCATION, expr);

    auto scope = state->scopes.curr;
    state->post_process_callbacks.push_back([scope, node, expr, sym,
                                             expected_type]() -> bool {
        if (expr == nullptr) {
            if (!type::is_nil(expected_type)) {
                INVALID_RETURN_TYPE_ERROR(
                    node->location, sym->id,
                    sym->type->value.function->return_type, expected_type);
                return false;
            }
            return true;
        }
        auto expr_type = lookup_node_type(scope, expr);
        if (!type::is_convertible(expr_type, expected_type)) {
            INVALID_RETURN_TYPE_ERROR(node->location, sym->id,
                                      sym->type->value.function->return_type,
                                      expr_type);
            return false;
        }
        return true;
    });
    add_instruction(state, node);
}

node::Node *new_arithmetic_operation(State *state, node::Node *lhs,
                                     node::Node *rhs,
                                     node::ArithmeticOperationKind kind,
                                     size_t line,
                                     std::string const &operator_name) {
    // if one of the operand is undefined, the node should always have the nill
    // type
    auto op_node = node::create_arithmetic_operation(LOCATION, kind, lhs, rhs);
    auto scope = state->scopes.curr;
    state->post_process_callbacks.push_back([op_node, scope, lhs, rhs,
                                             operator_name]() -> bool {
        auto lhs_type = lookup_node_type(scope, lhs);
        auto rhs_type = lookup_node_type(scope, rhs);

        if (!type::supports_arithmetic(lhs_type) ||
            !type::supports_arithmetic(rhs_type)) {
            ARITHMETIC_OPERATOR_ERROR(op_node->location, operator_name)
            return false;
        }
        scope->node_types.insert(
            {op_node, type::select_most_precise_arithmetic_type(lhs_type,
                                                                rhs_type)});
        return true;
    });
    return op_node;
}

void save_function_call_argument(State *state, node::Node *node) {
    state->funcall_parameters.back().push_back(node);
}

void begin_new_funcall(State *state) {
    state->funcall_parameters.push_back({});
}

node::Node *new_function_call(State *state, std::string const &function_name,
                              size_t line) {
    auto args = state->funcall_parameters.back();
    auto *node = node::create_function_call(LOCATION, function_name, args);
    state->funcall_parameters.pop_back();

    // setup type verification
    std::string file = state->curr_filename;
    auto scope = state->scopes.curr;
    state->post_process_callbacks.push_back([scope, node, args]() -> bool {
        auto function_name = node->value.function_call->name;
        auto sym = lookup_id(scope, function_name);

        if (sym == nullptr) {
            UNDEFINED_SYMBOL_ERROR(node->location, function_name);
            return false;
        }
        if (sym->type->kind != type::TypeKind::Function) {
            INVALID_CALL_ERROR(node->location, function_name);
            return false;
        }

        auto function_type = sym->type->value.function;
        scope->node_types.insert({node, function_type->return_type});
        if (function_type->arguments_types.size() != args.size()) {
            WRONG_NUMBER_OF_ARGUMENT_ERROR(node->location, function_name,
                                           sym->type);
            return false;
        }

        bool res = true;
        auto args_types = function_type->arguments_types;
        for (size_t idx = 0; idx < args.size(); ++idx) {
            auto found_type = lookup_node_type(scope, args[idx]);
            auto expected_tpe = args_types[idx];

            if (!type::is_convertible(found_type, expected_tpe)) {
                ARGUMENT_TYPE_ERROR(node->location, function_name, idx,
                                    found_type, expected_tpe);
                res = false;
            }
        }
        return res;
    });
    return node;
}

void new_variable_declaration(State *state, std::string id, type::Type *type,
                              size_t line) {
    auto symbol = state->scopes.curr->symbols.find(id);
    auto location = location_create(state->curr_filename, line);

    // redefinitions are not allowed:
    if (symbol != state->scopes.curr->symbols.end()) {
        MULTIPLE_DEFINITION_ERROR(location, id, symbol->second.location)
        state->status = 1;
    }
    insert_symbol(state->scopes.curr, id, type, nullptr, location);
    add_instruction(state, node::create_variable_definition(location, id));
}

node::Node *new_variable_reference(State *state, std::string const &name,
                                   size_t line) {
    auto sym = lookup_id(state->scopes.curr, name);
    auto node = node::create_variable_reference(LOCATION, name);

    if (!sym) {
        UNDEFINED_SYMBOL_ERROR(node->location, name);
        state->scopes.curr->node_types.insert({node, type::create_nil_type()});
        state->status = 1;
    } else {
        state->scopes.curr->node_types.insert({node, sym->type});
    }
    return node;
}

node::Node *new_index_expr(State *state, std::string const &name, size_t line,
                           node::Node *index_node) {
    auto location = location_create(state->curr_filename, line);
    auto variable = node::create_variable_reference(location, name);
    auto node = node::create_index_expression(location, variable, index_node);
    auto sym = lookup_id(state->scopes.curr, name);

    if (!sym) {
        UNDEFINED_SYMBOL_ERROR(node->location, name);
        state->status = 1;
        state->scopes.curr->node_types.insert({node, type::create_nil_type()});
        return node;
    } else if (sym->type->kind != type::TypeKind::Array) {
        INDEX_NON_ARRAY_TYPE_ERROR(node->location, name);
        state->status = 1;
        state->scopes.curr->node_types.insert({node, sym->type});
        state->scopes.curr->node_types.insert({variable, sym->type});
    } else {
        state->scopes.curr->node_types.insert(
            {node, sym->type->value.array->type});
        state->scopes.curr->node_types.insert({variable, sym->type});
        auto scope = state->scopes.curr;
        state->post_process_callbacks.push_back([scope, index_node]() -> bool {
            auto index_type = lookup_node_type(scope, index_node);
            if (!type::is_int(index_type)) {
                INVALID_INDEX_TYPE_ERROR(index_node->location, index_type);
                return false;
            }
            return true;
        });
    }
    return node;
}

void new_assignment(State *state, node::Node *target, node::Node *expr,
                    size_t line) {
    auto location = LOCATION;
    auto node = node::create_assignment(location, target, expr);
    auto variable_name = get_lvalue_identifier(target);
    auto symbol = lookup_id(state->scopes.curr, variable_name);

    add_instruction(state, node);

    if (!symbol) {
        UNDEFINED_SYMBOL_ERROR(location, variable_name);
        state->status = 1;
        return;
    }

    auto target_type = lookup_node_type(state->scopes.curr, target);
    if (!type::is_primitive(target_type)) {
        INVALID_MOV_ERROR(location, target_type);
        state->status = 1;
        return;
    }

    auto scope = state->scopes.curr;
    state->post_process_callbacks.push_back([scope, node, target_type, expr]()
            -> bool {
        auto expr_type = lookup_node_type(scope, expr);
        // TOOD: warning?
        if (!type::is_convertible(expr_type, target_type)) {
            BAD_ASSIGNMENT_ERROR(node->location, expr_type, target_type);
            return false;
        }
        if (!type::equal(expr_type, target_type)) {
            IMPLICIT_CONVERTION_WARNING(node->location, expr_type, target_type);
        }
        return true;
    });
}

node::Node *new_for(State *state, std::string const &index_id,
                    node::Node *begin, node::Node *end, node::Node *step,
                    node::Block *block, size_t line) {
    auto sym = lookup_id(state->scopes.curr, index_id);
    auto var_node = node::create_variable_reference(LOCATION, index_id);

    if (!sym) {
        UNDEFINED_SYMBOL_ERROR(var_node->location, index_id);
        state->status = 1;
    } else if (!type::supports_arithmetic(sym->type)) {
        BAD_FOR_INDEX_VARIABLE_TYPE(var_node->location, index_id, sym->type);
    } else {
        auto scope = state->scopes.curr;
        state->post_process_callbacks.push_back(
            [scope, var_node, sym, begin, step]() -> bool {
                auto begin_type = lookup_node_type(scope, begin);
                auto step_type = lookup_node_type(scope, step);
                bool res = true;

                if (!type::is_convertible(begin_type, sym->type)) {
                    FOR_RNG_ERROR(var_node->location, "begin expression",
                                  sym->id, sym->type, begin_type);
                    res = false;
                }
                if (!type::is_convertible(step_type, sym->type)) {
                    FOR_RNG_ERROR(var_node->location, "step expression",
                                  sym->id, sym->type, step_type);
                    res = false;
                }
                return res;
            });
    }
    leave_scope(state, block);
    return node::create_for_stmt(var_node->location, var_node, begin, end, step,
                                 block);
}

void new_shw(State *state, node::Node *expr, size_t line) {
    auto scope = state->scopes.curr;
    auto location = LOCATION;
    auto node = node::create_builtin_function(
        location, node::BuiltinFunctionKind::Shw, expr);

    state->post_process_callbacks.push_back([scope, node, expr]() -> bool {
        auto expr_type = lookup_node_type(scope, expr);
        if (!type::is_str(expr_type)) {
            ERROR(node->location, "shw takes a string as argument.");
            return false;
        }
        // TODO: this may change if in the future implementation of shw
        // support variable display
        return true;
    });
    add_instruction(state, node);
}

bool try_verify_main_type(State *state) {
    auto sym = lookup_id(state->scopes.global, "main");

    if (!sym) {
        return true; // when compiling libraries or object files
    }

    if (sym->type->kind != type::TypeKind::Function) {
        ERROR(sym->location, "main should be a function.")
        return false;
    }

    if (!type::is_int(sym->type->value.function->return_type)) {
        ERROR(sym->location, "invalid return type for main.")
        return false;
    }

    return true;
}

} // end namespace s3c
