#include "preprocessor.hpp"
#include <algorithm>
#include <regex>

/**
 * @brief  Get the path to the project and launch the preprocessor.
 *
 * @param  pathToMain  Just the path to the file given to the transpiler.
 */
void Preprocessor::process(std::string pathToMain) {
        std::string tmp = pathToMain;

        while (tmp[tmp.length() - 1] != '/')
                tmp.pop_back();
        pathToProject = tmp;
        process_rec(pathToMain);
}

/**
 * @brief  Follow all the includes statemnents in the file and write all the
 *         code in one file that will be parsed and transpiled.
 *
 * @param  fileName  Name of the file to process.
 */
void Preprocessor::process_rec(std::string fileName) {
        int lineCount = 1;
        std::string line;
        std::string includedFileName;
        std::regex includeStmt("^include .+;$");
        std::regex fileIndicator("-->.*");
        std::ifstream currentFile;

        // close the previously opened file (recursive call)
        filesStack.push_back(fileName);
        currentFile.open(fileName);

        // file indicator for the parser
        outputFile << "-->" << fileName << "-0" << std::endl;

        while (std::getline(currentFile, line)) {
                // if the user tries to add a file indicator, we make
                // sure that the program is not parsed
                if (std::regex_match(line, fileIndicator)) {
                        std::ostringstream oss;
                        oss << fileName << ":" << lineCount
                            << ": synctax error." << std::endl;
                        throw std::logic_error(oss.str());
                }
                // search for include statement
                if (std::regex_match(line, includeStmt)) {
                        includedFileName = pathToProject +
                                           line.substr(8, line.length() - 9) +
                                           ".prog";
                        bool isTreated =
                            std::find(treatedFiles.begin(), treatedFiles.end(),
                                      includedFileName) != treatedFiles.end();
                        bool isInStack =
                            std::find(filesStack.begin(), filesStack.end(),
                                      includedFileName) != filesStack.end();
                        if (!isTreated && !isInStack) {
                                // treat the file
                                process_rec(includedFileName);
                                outputFile << "-->" << fileName << "-"
                                           << (lineCount - 1) << std::endl;
                        }
                        outputFile << "# " << line << std::endl;
                } else {
                        // put the line in the output file
                        outputFile << line << std::endl;
                }
                lineCount++;
        }
        // when it's done, the file is considered as treated
        treatedFiles.push_back(filesStack.back());
        filesStack.pop_back();
        currentFile.close();
}
