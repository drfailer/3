#ifndef PROGRAMBUILDER_H
#define PROGRAMBUILDER_H
#include "ast/program.hpp"
#include <memory>

// TODO: cette classe doit être utilisée pour construire l'abre avec le parser.
// elle doit contenir des piles tampons pour pouvoir ajouter les opération, les
// block, les statements, ... quand le parser les trouves. Les éléments en haut
// de l'arbre sont construit en derniers ! => on doit pouvoir créer une liste de
// commandes, qu'on peut combiner dans un block, block qu'on peut ajouter à un
// statement qui sera lui aussi ajouter à une liste pour pouvoir créer une
// fonction ou être ajouter à un autre block.
class ProgramBuilder {
  public:
    ProgramBuilder();

    std::list<Variable> const &getFunParams() const;
    std::shared_ptr<Program> getProgram() const;
    type_system::types getParamsTypes() const;

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

    void currFunctionReturnType(type_system::type currFunctionReturnType) {
        currFunctionReturnType_ = currFunctionReturnType;
    }

    type_system::type const currFunctionReturnType() const {
        return currFunctionReturnType_;
    }

    void currFunctionName(std::string const &currFunctionName) {
        currFunctionName_ = currFunctionName;
    }
    std::string const &currFunctionName() { return currFunctionName_; }

    /* create functions *******************************************************/

    std::shared_ptr<FunctionCall> createFuncall(
        type_system::type = type_system::make_type<type_system::None>());

    std::shared_ptr<Cnd> createCnd(std::shared_ptr<Node>,
                                   std::shared_ptr<Block>);
    std::shared_ptr<For> createFor(Variable, std::shared_ptr<Node>,
                                   std::shared_ptr<Node>, std::shared_ptr<Node>,
                                   std::shared_ptr<Block>);
    std::shared_ptr<Whl> createWhl(std::shared_ptr<Node>,
                                   std::shared_ptr<Block>);

    void pushFuncallParam(std::shared_ptr<TypedNode>);
    void pushFunctionParam(Variable);
    void newFuncall(std::string);

    void createFunction(std::string, std::shared_ptr<Block>, type_system::type);

  private:
    std::string currFileName_;
    type_system::type currFunctionReturnType_ = nullptr;
    std::string currFunctionName_;
    std::shared_ptr<Program> program = nullptr;
    // todo: use std::stack
    std::list<std::shared_ptr<Block>> blocks = {};
    std::list<Variable> funParams = {};
    std::list<std::list<std::shared_ptr<TypedNode>>> funcallParams = {};
    // NOTE: maybe move this to the .y file as global variable:
    std::list<std::string> funcallIds = {};
};

#endif
