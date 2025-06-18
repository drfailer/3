#ifndef TYPE_PREDICATES
#define TYPE_PREDICATES
#include "type.hpp"

namespace type {

bool equal(Type const *t1, Type const *t2);
bool is_convertible(Type const *from, Type const *to);

bool is_primitive(Type const *type);

bool is_number(Type const *type);
bool supports_arithmetic(Type *type);

} // end namesapce type

#endif
