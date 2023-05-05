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
  std::string line;
  std::string includedFileName;
  std::regex  includeStmt("^include .+;$");

  // close the previously opened file (recursive call)
  if (currentFile.is_open())
    currentFile.close();
  filesStack.push_back(fileName);
  currentFile.open(fileName);

  while (std::getline(currentFile, line)) {
    // search for include statement
    if (std::regex_match(line, includeStmt)) {
      includedFileName =
          pathToProject + line.substr(8, line.length() - 9) + ".prog";
      bool isTreated = std::find(treatedFiles.begin(), treatedFiles.end(),
                                 includedFileName) != treatedFiles.end();
      bool isInStack = std::find(filesStack.begin(), filesStack.end(),
                                 includedFileName) != filesStack.end();
      if (!isTreated && !isInStack) {
        // treat the file
        process_rec(includedFileName);
        currentFile.open(filesStack.back());
      }
    } else {
      // put the line in the output file
      outputFile << line << std::endl;
    }
  }
  // when it's done, the file is considered as treated
  treatedFiles.push_back(filesStack.back());
  filesStack.pop_back();
  currentFile.close();
}
