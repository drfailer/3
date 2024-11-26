#ifndef PROGRAMBUILDER_H
#define PROGRAMBUILDER_H
#include "ast/program.hpp"
#include <memory>
#include <stack>

class ProgramBuilder {
  public:
    ProgramBuilder();

    // TODO: change this
    std::list<Variable> const &functionParameters() const;
    std::shared_ptr<Program> program() const;
    type_system::types_t parametersTypes() const;

    void display();

    void beginBlock(); // create an empty block on the top of the blocks stack
    std::shared_ptr<Block> endBlock(); // pop the last block of the blocks stack
    void pushBlock(std::shared_ptr<Node>); // add command to the last block

    void currFileName(std::string const &fileName) {
        // the filename is surrounded by '"' (we remove them)
        currFileName_ = fileName.substr(1);
        currFileName_.pop_back();
    }

    std::string const &currFileName() const { return currFileName_; }

    void currFunctionName(std::string const &currFunctionName) { currFunctionName_ = currFunctionName; }
    std::string const &currFunctionName() { return currFunctionName_; }

    /* create functions *******************************************************/

    std::shared_ptr<FunctionCall> createFuncall(type_system::type_t = type_system::make_type<type_system::None>());
    std::shared_ptr<Cnd> createCnd(std::shared_ptr<Node>, std::shared_ptr<Block>);
    std::shared_ptr<For> createFor(Variable, std::shared_ptr<Node>, std::shared_ptr<Node>, std::shared_ptr<Node>, std::shared_ptr<Block>);
    std::shared_ptr<Whl> createWhl(std::shared_ptr<Node>, std::shared_ptr<Block>);

    void pushFuncallParam(std::shared_ptr<TypedNode>);
    void pushFunctionParam(Variable);
    void newFuncall(std::string);

    void createFunction(std::string, std::shared_ptr<Block>, type_system::type_t);

  private:

    // TODO: split the tasks of managing funcalls, functions, ...
    std::string currFileName_;
    std::string currFunctionName_;
    std::shared_ptr<Program> program_ = nullptr;
    std::stack<std::shared_ptr<Block>> blocks = {};
    std::list<Variable> currFunctionParameters_ = {};
    std::list<std::list<std::shared_ptr<TypedNode>>> functionCallsParameters_ = {};
    std::list<std::string> funcallIds_ = {};
};

#endif
