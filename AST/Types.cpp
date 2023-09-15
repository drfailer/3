#include "Types.hpp"

std::string typeToString(Type type) {
        std::string output;

        switch (type) {
        case INT:
                output = "int";
                break;
        case FLT:
                output = "flt";
                break;
        case CHR:
                output = "chr";
                break;
        default:
                output = "void";
                break;
        }
        return output;
}
