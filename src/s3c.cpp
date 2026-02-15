#include "s3c.hpp"
#include "symbol_table.hpp"
#include "tools/messages.hpp"
#include "tools/type_utilities.hpp"
#include "ast.hpp"
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
    state->arena = arena_create();
    state->allocator = arena_allocator(&state->arena);
    mem_pool_init(&state->ast_pool, 100);
    return state;
}

void state_destroy(State *state) {
    mem_pool_destroy(&state->ast_pool);
    arena_destroy(&state->arena);
    delete state;
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

void leave_scope(State *state, Ast *block) {
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

Ast *new_argument_declaration(State *state, std::string const &id,
                                     type::Type *type, size_t line) {
    auto location = location_create(state->curr_filename, line);
    auto ast = new_ast(&state->ast_pool, location, AstKind::VariableDefinition,
                         .variable_definition = VariableDefinition{
                            .name = string_create(id, state->allocator),
                         });
    add_symbol(state, id, type, location);
    state->curr_function.arguments.push_back(ast);
    state->curr_function.arguments_types.push_back(type);
    return ast;
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
                             Ast *body, size_t line) {
    state->program.push_back(new_ast(&state->ast_pool, LOCATION, AstKind::FunctionDefinition,
                             .function_definition = FunctionDefinition{
                                .name = string_create(name, state->allocator),
                                .arguments = array_create_from_std_vector(
                                        state->curr_function.arguments,
                                        state->allocator),
                                .body = body,
                            }));
    leave_scope(state, body);
    reset_curr_function(state);
}

void add_function_declaration(State *state, size_t line) {
    state->program.push_back(new_ast(&state->ast_pool, LOCATION,AstKind::FunctionDeclaration,
                             .function_declaration = FunctionDeclaration{
                                .name = string_create(state->curr_function.name, state->allocator),
                                .arguments = array_create_from_std_vector(
                                        state->curr_function.arguments,
                                        state->allocator),
                            }));
    s3c::leave_scope(state, nullptr);
    s3c::reset_curr_function(state);
}

void begin_block(State *state) {
    state->curr_function.blocks.push(new_ast(
                &state->ast_pool,
                Location{},
                AstKind::Block,
                .block = { array_create<Ast*>(0, 0, state->allocator) }));
}

Ast *end_block(State *state) {
    Ast *lastBlock = state->curr_function.blocks.top();
    state->curr_function.blocks.pop();
    return lastBlock;
}

void add_instruction(State *state, Ast *ast) {
    array_append(&state->curr_function.blocks.top()->data.block.asts, ast);
}

void new_return_expr(State *state, Ast *expr, size_t line) {
    std::ostringstream oss;
    Symbol *sym = lookup_id(state->scopes.curr, state->curr_function.name);
    auto ast = new_ast(&state->ast_pool, LOCATION, AstKind::RetStmt,
                         .ret_stmt = { expr });
    add_instruction(state, ast);
}

Ast *new_arithmetic_operation(State *state, Ast *lhs,
                                     Ast *rhs,
                                     ArithmeticOperationKind kind,
                                     size_t line,
                                     std::string const &operator_name) {
    // if one of the operand is undefined, the ast should always have the nill
    // type
    auto op_ast = new_ast(&state->ast_pool, LOCATION, AstKind::ArithmeticOperation,
                           .arithmetic_operation = ArithmeticOperation{ kind, lhs, rhs });
    return op_ast;
}

void save_function_call_argument(State *state, Ast *ast) {
    state->funcall_parameters.back().push_back(ast);
}

void begin_new_funcall(State *state) {
    state->funcall_parameters.push_back({});
}

Ast *new_function_call(State *state, std::string const &function_name,
                              size_t line) {
    auto args = state->funcall_parameters.back();
    auto ast = new_ast(&state->ast_pool, LOCATION, AstKind::FunctionCall,
                         .function_call = FunctionCall{
                            .name = string_create(function_name, state->allocator),
                            .arguments = array_create_from_std_vector(args, state->allocator),
                         });
    state->funcall_parameters.pop_back();
    return ast;
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
    add_instruction(state, new_ast(&state->ast_pool, location, AstKind::VariableDefinition,
                         .variable_definition = VariableDefinition{
                            .name = string_create(id, state->allocator),
                         }));
}

Ast *new_variable_reference(State *state, std::string const &name,
                                   size_t line) {
    auto sym = lookup_id(state->scopes.curr, name);
    auto ast = new_ast(&state->ast_pool, LOCATION, AstKind::VariableReference,
                         .variable_reference = VariableReference{
                              .name = string_create(name, state->allocator),
                         });

    if (!sym) {
        UNDEFINED_SYMBOL_ERROR(ast->location, name);
        state->scopes.curr->ast_types.insert({ast, type::create_nil_type()});
        state->status = 1;
    } else {
        state->scopes.curr->ast_types.insert({ast, sym->type});
    }
    return ast;
}

Ast *new_index_expr(State *state, std::string const &name, size_t line,
                           Ast *index_ast) {
    auto location = location_create(state->curr_filename, line);
    auto variable = new_ast(&state->ast_pool, LOCATION, AstKind::VariableReference,
                             .variable_reference = VariableReference{
                                .name = string_create(name, state->allocator),
                             });
    auto ast = new_ast(&state->ast_pool, location, AstKind::IndexExpression,
                         .index_expression = IndexExpression{
                            .element = variable,
                            .index = index_ast,
                         });
    auto sym = lookup_id(state->scopes.curr, name);

    if (!sym) {
        UNDEFINED_SYMBOL_ERROR(ast->location, name);
        state->status = 1;
        state->scopes.curr->ast_types.insert({ast, type::create_nil_type()});
        return ast;
    } else if (sym->type->kind != type::TypeKind::Array) {
        INDEX_NON_ARRAY_TYPE_ERROR(ast->location, name);
        state->status = 1;
        state->scopes.curr->ast_types.insert({ast, sym->type});
        state->scopes.curr->ast_types.insert({variable, sym->type});
    } else {
        state->scopes.curr->ast_types.insert(
            {ast, sym->type->value.array->type});
        // this may not be useful
        state->scopes.curr->ast_types.insert({variable, sym->type});
    }
    return ast;
}

Ast *new_assignment(State *state, Ast *target, Ast *expr,
                           size_t line) {
    auto location = LOCATION;
    auto ast = new_ast(&state->ast_pool, location, AstKind::Assignment,
                         .assignment = Assignment{ target, expr });
    auto variable_name = get_lvalue_identifier(target);
    auto symbol = lookup_id(state->scopes.curr, variable_name);

    if (!symbol) {
        UNDEFINED_SYMBOL_ERROR(location, variable_name);
        state->status = 1;
        return ast;
    }

    auto target_type = lookup_ast_type(state->scopes.curr, target);
    if (!type::is_primitive(target_type)) {
        INVALID_MOV_ERROR(location, target_type);
        state->status = 1;
        return ast;
    }
    return ast;
}

void new_cnd(State *state, Ast *cond, size_t line) {
    state->parser_stack.push(new_ast(&state->ast_pool, location_create(state->curr_filename, line),
                AstKind::CndStmt, .cnd_stmt = CndStmt{
                    .condition = cond,
                    .block = nullptr,
                    .otw = nullptr,
                }));
}

void new_otw(State *state) {
    auto block = s3c::end_block(state);
    s3c::leave_scope(state, block);

    CndStmt *cur = &state->parser_stack.top()->data.cnd_stmt;
    while (cur->block != nullptr) {
        cur = &cur->otw->data.cnd_stmt;
    }
    cur->block = block;

    s3c::begin_block(state);
    s3c::enter_scope(state);
}

void new_otw_cnd(State *state, Ast *cond, size_t line) {
    CndStmt *cur = &state->parser_stack.top()->data.cnd_stmt;
    while (cur->otw != nullptr) {
        cur = &cur->otw->data.cnd_stmt;
    }
    cur->otw = new_ast(&state->ast_pool, location_create(state->curr_filename, line),
                        AstKind::CndStmt, .cnd_stmt = CndStmt{
                            .condition = cond,
                            .block = nullptr,
                            .otw = nullptr,
                        });
}

Ast *end_cnd(State *state) {
    auto cnd = state->parser_stack.top();
    auto block = s3c::end_block(state);
    s3c::leave_scope(state, block);

    Ast *cur = cnd;
    while (cur->data.cnd_stmt.otw != nullptr) {
        cur = cur->data.cnd_stmt.otw;
    }
    if (cur->data.cnd_stmt.block == nullptr) {
        cur->data.cnd_stmt.block = block;
    } else {
        block->location = cur->location;
        cur->data.cnd_stmt.otw = block;
    }
    state->parser_stack.pop();
    return cnd;
}

Ast *new_for(State *state, Ast *init, Ast *end,
                    Ast *step, Ast *block, size_t line) {
    auto location = LOCATION;
    leave_scope(state, block);
    // the syntax allow to just put the expression that is assigned to the loop
    // variable, therefore, we need to manually create the assignment
    auto step_assignment =
        new_assignment(state, init->data.assignment.target, step, line);
    return new_ast(&state->ast_pool, location, AstKind::ForStmt,
                    .for_stmt = ForStmt{
                        .init = init,
                        .condition = end,
                        .step = step_assignment,
                        .block = block,
                    });
}

void new_shw(State *state, Ast *expr, size_t line) {
    auto location = LOCATION;
    auto ast = new_ast(&state->ast_pool, location, AstKind::BuiltinFunction,
                         .builtin_function = BuiltinFunction{
                            .kind = BuiltinFunctionKind::Shw,
                            .argument = expr,
                         });
    add_instruction(state, ast);
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
