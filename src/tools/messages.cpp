#include "messages.hpp"
#include <iostream>

namespace msg {

void error(std::string const &msg) {
    std::cerr << "[" << ERR << "ERROR" << NORM << "]: " << msg << std::endl;
}

void warning(std::string const &msg) {
    std::cerr << "[" << WARN << "WARN" << NORM << "]: " << msg << std::endl;
}

} // end namespace msg
