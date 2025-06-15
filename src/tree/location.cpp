#include "location.hpp"

Location create_location(std::string const &filename, size_t row, size_t col) {
    return Location{
        .row = row,
        .col = col,
        .filename = filename,
    };
}

std::ostream &operator<<(std::ostream &os, Location const &location) {
    os << location.filename << ':' << location.row << ':' << location.col;
    return os;
}
