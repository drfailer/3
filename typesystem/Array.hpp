#ifndef ARRAY_HPP
#define ARRAY_HPP
#include "typesystem/Type.hpp"

class Array: public Type {
      public:
        int getDimension() const { return dimension; }
        Array(Atomic type, int dimension): Type(type), dimension(dimension) {}

      private:
        int dimension;
};

#endif
