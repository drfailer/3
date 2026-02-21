#ifndef TOOLS_MESSAGES
#define TOOLS_MESSAGES
#include "type.hpp"
#include <sstream>
#include <string>

#define ERR "\033[1;31m"
#define WARN "\033[1;33m"
#define BOLD "\033[1;34m"
#define NORM "\033[0m"

namespace msg {

void error(std::string const &msg);
void warning(std::string const &msg);

} // end namespace msg

#define ERROR(loc, args)                                                       \
    {                                                                          \
        std::ostringstream oss;                                                \
        oss << loc << ": " << args;                                            \
        msg::error(oss.str());                                                 \
    }
#define WARNING(loc, args)                                                     \
    {                                                                          \
        std::ostringstream oss;                                                \
        oss << loc << ": " << args;                                            \
        msg::warning(oss.str());                                               \
    }
#define QUOTE(val) "'" BOLD << val << NORM "'"

#define UNDEFINED_SYMBOL_ERROR(loc, symbol_id)                                 \
    ERROR(loc, "undefined symbol " << QUOTE(symbol_id) << ". " << __FILE__ << ":" << __LINE__)
#define MULTIPLE_DEFINITION_ERROR(loc, name, symbol_location)                  \
    ERROR(loc, "redifinition definition of symbol "                            \
                   << QUOTE(name) << " (previously defined here: '"            \
                   << QUOTE(symbol_location) << "').")
// TODO: this message could be improved
#define ARITHMETIC_OPERATOR_ERROR(loc, operator_name)                          \
    ERROR(loc, "bad usage of operator " << QUOTE(operator_name) << ".")
#define INVALID_CALL_ERROR(loc, id)                                            \
    ERROR(loc, "invalide call to non function object " << QUOTE(id) << ".")
// TODO: need a function to print type list
#define NON_MATCHING_FUNCTION_CALL_ERROR(loc, function_name, found, expected)  \
    ERROR(loc, "non matching funciton call to "                                \
                   << QUOTE(function_name)                                     \
                   << ".\n"                                                    \
                      "found: "                                                \
                   << QUOTE(type_to_string(found))                       \
                   << ".\n"                                                    \
                      "expected: "                                             \
                   << QUOTE(type_to_string(expected)));
#define INVALID_RETURN_TYPE_ERROR(loc, function_id, found_type, expected_type) \
    ERROR(loc, "invalid return type found in "                                 \
                   << QUOTE(function_id)                                       \
                   << ".\n"                                                    \
                      "found: "                                                \
                   << QUOTE(type_to_string(found_type))                  \
                   << "\n."                                                    \
                      "expected: "                                             \
                   << QUOTE(type_to_string(expected_type)))
#define WRONG_NUMBER_OF_ARGUMENT_ERROR(loc, function_name, function_type)      \
    ERROR(loc, "wrong number of argument for "                                 \
                   << QUOTE(function_name) << " which is of type "             \
                   << QUOTE(type_to_string(function_type)) << ".\n")

#define ARGUMENT_TYPE_ERROR(loc, function_name, arg_idx, found_type,           \
                            expected_type)                                     \
    ERROR(loc,                                                                 \
          "wrong type for argument "                                           \
              << arg_idx << " of function " << QUOTE(function_name) << ".\n"   \
              << "found: " << QUOTE(type_to_string(found_type)) << ".\n" \
              << "expected: " << QUOTE(type_to_string(expected_type)))

#define INDEX_NON_ARRAY_TYPE_ERROR(loc, variable_name)                         \
    ERROR(loc, QUOTE(variable_name) << " is not indexable.")
#define INVALID_INDEX_TYPE_ERROR(loc, found_type)                              \
    ERROR(loc, "index type should be "                                         \
                   << QUOTE("int") << " found "                                \
                   << QUOTE(type_to_string(found_type)) << ".")

#define IMPLICIT_CONVERTION_WARNING(loc, found_type, expected_type)            \
    WARNING(loc, "implicit convertion from "                                   \
                     << QUOTE(type_to_string(found_type)) << " to "      \
                     << QUOTE(type_to_string(expected_type)) << ".")

#define INVALID_MOV_ERROR(loc, target_type)                                    \
    ERROR(loc, "bad assignment of to expression of type "                      \
                   << QUOTE(type_to_string(target_type)) << ".")
#define BAD_ASSIGNMENT_ERROR(loc, found_type, expected_type)                   \
    ERROR(loc, "bad assignment of of an expression of type "                   \
                   << QUOTE(type_to_string(found_type))                  \
                   << " to a variable of type "                                \
                   << QUOTE(type_to_string(expected_type)) << ".")

#define BAD_FOR_INDEX_VARIABLE_TYPE(loc, index_id, found_type)                 \
    ERROR(loc, "variable " << QUOTE(index_id) << " is of type "                \
                           << QUOTE(type_to_string(found_type))          \
                           << " which cannot be used in a for loops.")

#define FOR_STEP_TYPE_ERROR(loc, found_type, expected_type)                    \
    ERROR(loc, "in for statement, found step expression of type "              \
                   << QUOTE(type_to_string(found_type))                  \
                   << ", which is incompatible with the index type "           \
                   << QUOTE(type_to_string(expected_type)) << ".")
#define FOR_STEP_TYPE_WARNING(loc, found_type, expected_type)                  \
    ERROR(loc, "in for statement, implicit convertion from "                   \
                   << QUOTE(type_to_string(found_type)) << " to "        \
                   << QUOTE(type_to_string(expected_type))               \
                   << " on step.")

#endif
