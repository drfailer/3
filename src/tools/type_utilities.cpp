#include "type_utilities.hpp"

std::string get_lvalue_identifier(node::Node *target) {
    if (target->kind == node::NodeKind::VariableReference) {
        return target->value.variable_reference->name;
    } else if (target->kind == node::NodeKind::IndexExpression) {
        return get_lvalue_identifier(target->value.index_expression->element);
    }
    return "";
}
