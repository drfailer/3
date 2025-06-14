#ifndef TOOLS_TYPE_UTILITIES
#define TOOLS_TYPE_UTILITIES
#include "../ast/node.hpp"
#include "../symtable/symbol_table.hpp"
#include "../symtable/type.hpp"

std::string get_lvalue_identifier(node::Node *target);

struct eval_type_r {
    type::Type *type;
    enum {
        Success,
        PostprocessableSymbolFound,
        UndefinedSymbolError,
        InvalidOperandType,
        InvalidIndexedDataFound,
        NonEvaluableNodeFound,
    } status;
};
eval_type_r get_eval_type(node::Node *node, SymbolTable *scope);
type::Type *get_value_etype(node::Value *value_node);

#endif
