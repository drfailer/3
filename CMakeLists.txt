cmake_minimum_required(VERSION 3.25.2)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
add_compile_options(-Wall -Wextra -Wuninitialized -g)

project(transpiler)

# generate lexer and parser using flex and bison
add_custom_target(parser ALL bison main_cpp.y
                  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/main_cpp.y
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                  COMMENT "building parser")
add_custom_target(lexer ALL flex main_cpp.l
                  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/main_cpp.l
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                  COMMENT "building lexer")

add_executable(prog parser.cpp lexer.cpp
  AST/AST.cpp AST/Program.cpp AST/ProgramBuilder.cpp
  AST/Types.cpp symtable/Symtable.cpp symtable/Symbol.cpp
  symtable/ContextManager.cpp
  errorManager/ErrorManager.cpp
  preprocessor/preprocessor.cpp
  checks.cpp)
