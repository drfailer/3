#ifndef AST_LOCATION
#define AST_LOCATION
#include <cstddef>
#include <string>
#include <ostream>

struct Location {
    size_t row;
    size_t col;
    std::string filename;
};

Location create_location(std::string const &filename, size_t row,
                         size_t col = 0);
std::ostream &operator<<(std::ostream &os, Location const &location);

#endif
