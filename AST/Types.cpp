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
        case ARR_INT:
                output = "int[]";
                break;
        case ARR_FLT:
                output = "flt[]";
                break;
        case ARR_CHR:
                output = "chr[]";
                break;
        default:
                output = "void";
                break;
        }
        return output;
}

Type getArrayType(Type type) {
        switch (type) {
        case INT:
                return ARR_INT;
                break;
        case FLT:
                return ARR_FLT;
                break;
        case CHR:
                return ARR_CHR;
                break;
        default:
                return type;
                break;
        }
}

Type getValueType(Type type) {
        switch (type) {
        case ARR_INT:
                return INT;
                break;
        case ARR_FLT:
                return FLT;
                break;
        case ARR_CHR:
                return CHR;
                break;
        default:
                return type;
                break;
        }
}

bool isArray(Type type) {
        return type == ARR_INT || type == ARR_FLT || type == ARR_CHR;
}
