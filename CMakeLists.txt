cmake_minimum_required(VERSION 3.25.2)

project(s3c)

# configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_VERBOSE_MAKEFILE TRUE)

# compiler options
add_compile_options(-Wall -Wextra -Wuninitialized -g)

include_directories(src)

# generate lexer and parser using flex and bison
add_custom_target(parser COMMAND bison -Wconflicts-sr -Wcounterexamples parser_config.y
                  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/parser_config.y
                  BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/parser.cpp
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/
                  COMMENT "building parser")
add_custom_target(lexer COMMAND flex lexer_config.l
                  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/lexer_config.l
                  BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/lexer.cpp
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/
                  COMMENT "building lexer")

set(files
    src/compiler/compiler.cpp
    src/compiler/tools.cpp
    src/compiler/x86_64_gnu_linux.cpp
    src/main.cpp
    src/preprocessor/preprocessor.cpp
    src/s3c.cpp
    src/symbol_table.cpp
    src/tools/messages.cpp
    src/tools/string.cpp
    src/tools/mem.cpp
    src/tools/type_utilities.cpp
    src/tree/node.cpp
    src/tree/location.cpp
    src/type/predicates.cpp
    src/type/type.cpp
)

add_executable(s3c src/parser/parser.cpp src/parser/lexer.cpp ${files})
# add_dependencies(s3c parser lexer)
