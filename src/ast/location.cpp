#include "location.hpp"

Location create_location(std::string const &filename, size_t row, size_t col) {
    return Location{
        .row = row,
        .col = col,
        .filename = filename,
    };
}
