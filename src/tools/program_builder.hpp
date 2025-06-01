#ifndef PROGRAMBUILDER_H
#define PROGRAMBUILDER_H
#include "ast/node.hpp"
#include <list>
#include <stack>

struct ProgramBuilder {
    void display();

    void beginBlock(); // create an empty block on the top of the blocks stack
    node::Block *endBlock();      // pop the last block of the blocks stack
    void pushBlock(node::Node *); // add command to the last block

    void setCurrFileName(std::string const &fileName) {
        // the filename is surrounded by '"' (we remove them)
        currFileName = fileName[0] == '"' ? fileName.substr(1) : fileName;
        if (fileName.back() == '"') {
            currFileName.pop_back();
        }
    }

    /* create functions *******************************************************/

    node::Node *createFuncall();
    node::CndStmt *createCnd(node::Node *condition, node::Block *block);
    node::ForStmt *createFor(std::string const index_id, node::Node *start,
                             node::Node *end, node::Node *step,
                             node::Block *block);
    node::WhlStmt *createWhl(node::Node *condition, node::Block *block);

    void pushFuncallParam(node::Node *arg);
    void pushFunctionParam(std::string const &id);
    void newFuncall(std::string const &name);

    void createFunction(std::string const &name, node::Block *block);

    std::list<node::Node *> program;
    std::string currFileName;
    std::string currFunctionName;
    std::stack<node::Block *> blocks = {};
    std::list<std::string> currFunctionParameters = {};
    std::list<std::list<node::Node *>> functionCallsParameters = {};
    std::list<std::string> funcallIds = {};
};

#endif
