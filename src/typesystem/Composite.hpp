#ifndef COMPOSITE_HPP
#define COMPOSITE_HPP
#include "typesystem/Type.hpp"
#include <map>
#include <string>

/**
 * NOTE: composite types are not implemented yet.
 */

class Composite: public Type
{
public:
        Composite(): Type(COMPOSITE) {}
        ~Composite() = default;

private:
        std::map<std::string, Type> fields;
};

#endif
