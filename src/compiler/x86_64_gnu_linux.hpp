#ifndef COMPILER_X86_64_GNU_LINUX
#define COMPILER_X86_64_GNU_LINUX

#include "compiler/compiler.hpp"
#include "symbol_table.hpp"
#include "ast.hpp"

namespace compiler {

namespace x86_64 {

namespace gnu_linux {

void compile(CompilerState *state, Program const &program);

} // end namespace gnu_linux

} // end namespace x64_64

} // end namespace compiler

#endif
