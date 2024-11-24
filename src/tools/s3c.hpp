#ifndef TOOLS_S3C_H
#define TOOLS_S3C_H
#include "tools/program_builder.hpp"
#include "tools/errors_manager.hpp"
#include "symtable/context_manager.hpp"

class S3C {
  public:
    ProgramBuilder &programBuilder() { return programBuilder_; }
    Symtable &symtable() { return symtable_; }
    ContextManager &contextManager() { return contextManager_ ; }
    ErrorManager &errorsManager() { return errorsManager_; }

  private:
    ProgramBuilder programBuilder_;
    Symtable symtable_;
    ContextManager contextManager_;
    ErrorManager errorsManager_;
};

#endif
