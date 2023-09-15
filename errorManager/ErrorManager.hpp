#ifndef __ERROR_MANAGER__
#define __ERROR_MANAGER__
#include <iostream>
#include <sstream>

/* TODO:
 *
 * Create a Warining and Error class and find a way to sort the messages at the
 * end.
 */

class ErrorManager {
      public:
        bool getErrors() const;
        void newError(std::string);
        void newWarning(std::string);
        void report();
        ErrorManager() : errors(false) {}
        ~ErrorManager() = default;

      private:
        std::ostringstream errStream;
        bool errors;
};

#endif
