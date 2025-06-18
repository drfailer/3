#ifndef TOOLS_MESSAGES
#define TOOLS_MESSAGES
#include "../tree/location.hpp"
#include "type/type.hpp"
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
        oss << loc << ": " << args << std::endl;                               \
        msg::error(oss.str());                                                 \
    }
#define WARNING(loc, args)                                                     \
    {                                                                          \
        std::ostringstream oss;                                                \
        oss << loc << ": " << args << std::endl;                               \
        msg::warning(oss.str());                                               \
    }
#define QUOTE(val) "'" BOLD << val << NORM "'"

#define UNDEFINED_SYMBOL_ERROR(loc, symbol_id)                                 \
    ERROR(loc, "undefined symbol " << QUOTE(symbol_id) << ".")
#define MULTIPLE_DEFINITION_ERROR(loc, name, symbol_location)                  \
    ERROR(loc, "redifinition definition of symbol "                            \
                   << QUOTE(name) << " (previously defined here: '"            \
                   << QUOTE(symbol_location) << "').")
#define ARITHMETIC_OPERATOR_ERROR(loc, operator_name)                          \
    ERROR(loc, "bad usage of operator " << QUOTE(operator_name) << ".")
// TODO: need a function to print type list
#define NON_MATCHING_FUNCTION_CALL_ERROR(loc, function_name, found, expected)  \
    ERROR(loc, "non matching funciton call to "                                \
                   << QUOTE(function_name)                                     \
                   << ".\n"                                                    \
                      "found: "                                                \
                   << QUOTE(type::type_to_string(found))                       \
                   << ".\n"                                                    \
                      "expected: "                                             \
                   << QUOTE(type::type_to_string(expected)));
#define INVALID_RETURN_TYPE_ERROR(loc, function_id, found_type, expected_type) \
    ERROR(loc, "invalid return type found in "                                 \
                   << QUOTE(function_id)                                       \
                   << ".\n"                                                    \
                      "found: "                                                \
                   << QUOTE(type::type_to_string(found_type))                  \
                   << "\n."                                                    \
                      "expected: "                                             \
                   << QUOTE(type::type_to_string(expected_type)))
#define ARGUMENT_TYPE_ERROR(loc, function_name, arg_idx, found_type,           \
                            expected_type)                                     \
    TODO
#define INDEX_NON_ARRAY_TYPE_ERROR(loc, variable_name)                         \
    ERROR(loc, QUOTE(variable_name) << " is not indexable.")
#define INVALID_INDEX_TYPE_ERROR(loc, found_type)                              \
    ERROR(loc, "index type should be "                                         \
                   << QUOTE("int") << " found "                                \
                   << QUOTE(type::type_to_string(found_type)) << ".")

// TODO: useless untill `cnv` is implemented
#define IMPLICIT_CONVERTION_WARNING(loc, function_id, found_type,              \
                                    expected_type)                             \
    WARNING(loc, "in " << QUOTE(function_id) << " implicit convertion from "   \
                       << QUOTE(type::type_to_string(found_type)) << " to "    \
                       << QUOTE(expected_type) << ".")

#endif
