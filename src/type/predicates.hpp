#ifndef TYPE_PREDICATES
#define TYPE_PREDICATES
#include "type.hpp"

namespace type {

bool equal(Type const *t1, Type const *t2);
bool is_convertible(Type const *from, Type const *to);

bool is_primitive(Type const *type);
bool is_int(Type const *type);
bool is_chr(Type const *type);
bool is_flt(Type const *type);
bool is_str(Type const *type);
bool is_nil(Type const *type);

bool is_number(Type const *type);
bool supports_arithmetic(Type *type);
Type *select_most_precise_arithmetic_type(Type *lhs, Type *rhs);

} // end namesapce type

#endif
