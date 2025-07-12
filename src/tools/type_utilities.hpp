#ifndef TOOLS_TYPE_UTILITIES
#define TOOLS_TYPE_UTILITIES
#include "../tree/node.hpp"
#include "../symbol_table.hpp"
#include "../type/type.hpp"

// TODO: this functions should be moved in s3c and the get_eval_type should be
// exxpected to be used only when the symbol should be defined (therefore the
// PostprocessableSymbolFound status should be removed). Futhermore, moving this
// function in s3c will allow to print error messages in it which will be
// simpler than testing the return status everywhere. Note that all functions
// types should be post processed by default. Considering this, and the fact
// that the node types have been added in the symbol table, the implementation
// of get_eval_type should be way simpler (the function might even be removed).

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

#endif
