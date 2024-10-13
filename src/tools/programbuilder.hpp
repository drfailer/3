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

    std::list<Variable> getFunParams() const;
    std::shared_ptr<Program> getProgram() const;
    std::list<Type> getParamsTypes() const;

    void display();

    void beginBlock(); // create an empty block on the top of the blocks stack
    std::shared_ptr<Block> endBlock(); // pop the last block of the blocks stack
    void pushBlock(std::shared_ptr<Node>); // add command to the last block

    std::shared_ptr<FunctionCall> createFuncall();

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

    void createFunction(std::string, std::shared_ptr<Block>, Type);

  private:
    std::shared_ptr<Program> program = nullptr; // current program
    std::list<std::shared_ptr<Block>> blocks =
        {};                             // stack of blocks (the last
                                        // element is the current block)
    std::list<Variable> funParams = {}; // parameters of the last function
    std::list<std::list<std::shared_ptr<TypedNode>>> funcallParams =
        {}; // parameters of the last funcall
    // NOTE: maybe move this to the .y file as global variable:
    std::list<std::string> funcallIds = {};
    // std::shared_ptr<Function> currentFunction; // TODO: user this
};

#endif
