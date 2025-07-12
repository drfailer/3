#include "type_utilities.hpp"
#include <iostream>

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

type::Type *get_value_eval_type(node::Value *value_node) {
    switch (value_node->kind) {
    case node::ValueKind::Character:
        return type::create_primitive_type(type::PrimitiveType::Chr);
    case node::ValueKind::Integer:
        return type::create_primitive_type(type::PrimitiveType::Int);
    case node::ValueKind::Real:
        return type::create_primitive_type(type::PrimitiveType::Flt);
    case node::ValueKind::String:
        return type::create_primitive_type(type::PrimitiveType::Str);
    }
    return nullptr;
}

eval_type_r get_variable_eval_type(node::VariableReference *node,
                                   SymbolTable *scope) {
    auto symbol = lookup(scope, node->name);
    if (!symbol) {
        std::cerr << "error: use of undefined variable " << node->name << "."
                  << std::endl;
        return {nullptr, eval_type_r::UndefinedSymbolError};
    }
    return {symbol->type, eval_type_r::Success};
}

eval_type_r get_index_expression_eval_type(node::IndexExpression *node,
                                           SymbolTable *scope) {
    auto eval_type = get_eval_type(node->element, scope);

    if (eval_type.status != eval_type_r::Success) {
        return eval_type;
    }
    if (eval_type.type->kind != type::TypeKind::Array) {
        std::cerr << "error: the type " << type::type_to_string(eval_type.type)
                  << " is not indexable." << std::endl;
        return {nullptr, eval_type_r::InvalidIndexedDataFound};
    }
    return {eval_type.type->value.array->type, eval_type_r::Success};
}

eval_type_r get_arithmetic_operation_eval_type(node::ArithmeticOperation *node,
                                               SymbolTable *scope) {
    auto lhs_type = get_eval_type(node->lhs, scope);
    auto rhs_type = get_eval_type(node->rhs, scope);

    if (!lhs_type.type) {
        return lhs_type;
    }

    if (!rhs_type.type) {
        return rhs_type;
    }

    if (lhs_type.type->kind != type::TypeKind::Primitive ||
        rhs_type.type->kind != type::TypeKind::Primitive) {
        std::cerr << "error: cannot use arithmetic operator on a non "
                     "primitive type."
                  << std::endl;
        return {nullptr, eval_type_r::InvalidOperandType};
    }
    // Flt > Int so if one operand is of type Flt, the evaluated type is
    // Flt and not Int.
    if (lhs_type.type->value.primitive == type::PrimitiveType::Flt) {
        return lhs_type;
    }
    return rhs_type;
}

eval_type_r get_boolean_operation_eval_type(node::BooleanOperation *node,
        SymbolTable *scope) {
    // TODO: we should have a bool type for this
    // TODO: the type is always bool but we need a function that checks the type
    // of the operands
    std::cerr << "TODO: implement get_boolean_operation_eval_type" << std::endl;
    return {};
}

eval_type_r get_function_call_eval_type(node::FunctionCall *node,
                                        SymbolTable *scope) {
    auto symbol = lookup(scope, node->name);
    if (!symbol) {
        return {nullptr, eval_type_r::PostprocessableSymbolFound};
    }
    return {symbol->type->value.function->return_type, eval_type_r::Success};
}

eval_type_r get_eval_type(node::Node *node, SymbolTable *scope) {
    eval_type_r result;

    if (!node) {
        std::cerr << "warn[get_eval_type]: nullptr received." << std::endl;
        return {type::create_nil_type(), eval_type_r::Success};
    }

    // TODO: register node type

    switch (node->kind) {
    case node::NodeKind::Value:
        result = {get_value_eval_type(node->value.value), eval_type_r::Success};
        break;
    case node::NodeKind::VariableReference:
        result =  get_variable_eval_type(node->value.variable_reference, scope);
        break;
    case node::NodeKind::IndexExpression:
        result =  get_index_expression_eval_type(node->value.index_expression,
                                              scope);
        break;
    case node::NodeKind::ArithmeticOperation:
        result =  get_arithmetic_operation_eval_type(
            node->value.arithmetic_operation, scope);
        break;
    case node::NodeKind::BooleanOperation:
        result = get_boolean_operation_eval_type(node->value.boolean_operation,
                scope);
        break;
    case node::NodeKind::FunctionCall:
        result = get_function_call_eval_type(node->value.function_call, scope);
        break;
    default:
        std::cerr << "error: the given node is not evaluable." << std::endl;
        return {nullptr, eval_type_r::NonEvaluableNodeFound};
        break;
    }
    if (result.status == eval_type_r::Success) {
        scope->node_types[node] = result.type;
    }
    return result;
}
