#ifndef TOOLS_MESSAGES
#define TOOLS_MESSAGES
#include <string>

#define ERR "\033[1;31m"
#define WARN "\033[1;33m"
#define BOLD "\033[1;34m"
#define NORM "\033[0m"

namespace msg {

void error(std::string const &msg);
void warning(std::string const &msg);

} // end namespace msg

#endif
