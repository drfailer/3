cmake_minimum_required(VERSION 3.25.2)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
add_compile_options(-Wall -Wextra -Wuninitialized -g)

project(transpiler)

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
  src/ast/AST.cpp
  src/ast/Program.cpp
  src/ast/ProgramBuilder.cpp
  src/ast/Types.cpp
  src/symtable/Symtable.cpp
  src/symtable/Symbol.cpp
  src/symtable/ContextManager.cpp
  src/errorManager/ErrorManager.cpp
  src/preprocessor/preprocessor.cpp
  src/checks.cpp
)

add_executable(prog src/parser.cpp src/lexer.cpp ${files})
