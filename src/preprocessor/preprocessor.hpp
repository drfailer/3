#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/*
 * NOTE: the streams must be save in a stack, otherwise, files will be reopend
 * from the begining after each include.
 */

class Preprocessor {
      public:
        void process_rec(std::string fileName);
        void process(std::string pathToMain);
        Preprocessor(std::string outputFileName)
            : outputFile(std::ofstream(outputFileName)) {}
        ~Preprocessor() = default;

      private:
        std::vector<std::string> treatedFiles;
        std::vector<std::string> filesStack;
        std::ofstream outputFile;
        std::string pathToProject;
};

#endif
