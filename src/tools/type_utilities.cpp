#include "type_utilities.hpp"

std::string get_lvalue_identifier(Ast *target) {
    if (target->kind == AstKind::VariableReference) {
        return std::string(target->data.variable_reference.name.ptr);
    } else if (target->kind == AstKind::IndexExpression) {
        return get_lvalue_identifier(target->data.index_expression.element);
    }
    return "";
}
