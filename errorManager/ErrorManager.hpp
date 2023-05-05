#ifndef __ERROR_MANAGER__
#define __ERROR_MANAGER__
#include <sstream>
#include <iostream>

/* TODO:
 *
 * Create a Warining and Error class and find a way to sort the messages at the
 * end.
 */

class ErrorManager
{
  private:
    std::ostringstream errStream;
    bool errors;

  public:
    bool getErrors() const;
    void newError(std::string);
    void newWarning(std::string);
    void report();
    ErrorManager(): errors(false) {}
    ~ErrorManager() = default;
};

#endif
