#ifndef TOOLS_S3C_H
#define TOOLS_S3C_H
#include "tools/programbuilder.hpp"

class S3C {
  public:
    ProgramBuilder &programBuilder() { return programBuilder_; }

  private:
    ProgramBuilder programBuilder_;
};

#endif
