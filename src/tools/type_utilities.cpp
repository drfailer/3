#include "type_utilities.hpp"

std::string get_lvalue_identifier(node::Node *target) {
    if (target->kind == node::NodeKind::VariableReference) {
        return std::string(target->data.variable_reference.name.ptr);
    } else if (target->kind == node::NodeKind::IndexExpression) {
        return get_lvalue_identifier(target->data.index_expression.element);
    }
    return "";
}
