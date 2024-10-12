#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP
#include "typesystem/Type.hpp"
#include <vector>

class Function : public Type {
      public:
        std::vector<Type> getParameters() const { return parameters; }
        Function(Atomic type, std::vector<Type> parameters)
            : Type(type), parameters(parameters) {}
        ~Function() = default;

      private:
        std::vector<Type> parameters;
};

#endif
