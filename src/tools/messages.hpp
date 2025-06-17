#ifndef TOOLS_MESSAGES
#define TOOLS_MESSAGES
#include "../tree/location.hpp"
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

#define ERROR(args)                                                            \
    {                                                                          \
        std::ostringstream oss;                                                \
        oss << LOCATION << ": " << args << std::endl;                          \
        msg::error(oss.str());                                                 \
    }
#define WARNING(args)                                                          \
    {                                                                          \
        std::ostringstream oss;                                                \
        oss << LOCATION << ": " << args << std::endl;                          \
        msg::warning(oss.str());                                               \
    }

#define MULTIPLE_DEFINITION_ERROR(name, symbol_location)                       \
    ERROR("redifinition definition of symbol '"                                \
          << name << "' (previously defined here: '" << symbol_location        \
          << "').")

#endif
