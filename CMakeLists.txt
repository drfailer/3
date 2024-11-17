cmake_minimum_required(VERSION 3.25.2)

project(transpiler)

# configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# compiler options
add_compile_options(-Wall -Wextra -Wuninitialized -g)

include_directories(src)

# generate lexer and parser using flex and bison
add_custom_target(parser ALL bison -Wconflicts-sr -Wcounterexamples main_cpp.y
                  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/main_cpp.y
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
                  COMMENT "building parser")
add_custom_target(lexer ALL flex main_cpp.l
                  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/main_cpp.l
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
                  COMMENT "building lexer")

set(files
  src/ast/ast.cpp
  src/ast/program.cpp
  src/symtable/symtable.cpp
  src/symtable/contextmanager.cpp
  src/typesystem/types.cpp
  src/tools/errormanager.cpp
  src/tools/programbuilder.cpp
  src/tools/checks.cpp
  src/preprocessor/preprocessor.cpp
)

add_executable(s3c src/parser.cpp src/lexer.cpp ${files})
