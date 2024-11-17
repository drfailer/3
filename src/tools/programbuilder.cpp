#include "programbuilder.hpp"

ProgramBuilder::ProgramBuilder() : program(std::make_shared<Program>()) {}

void ProgramBuilder::display() { program->display(); }

/******************************************************************************/
/*                                   blocks                                   */
/******************************************************************************/

/**
 * @brief  Add `command` to the last block of the block stack.
 */
void ProgramBuilder::pushBlock(std::shared_ptr<Node> command) {
    blocks.back()->add(command);
}

/**
 * @brief  create an empty block on the top of the blocks stack
 */
void ProgramBuilder::beginBlock() {
    blocks.push_back(std::make_shared<Block>());
}

/**
 * @brief  pop the last block of the blocks stack
 */
std::shared_ptr<Block> ProgramBuilder::endBlock() {
    std::shared_ptr<Block> lastBlock = blocks.back();
    blocks.pop_back();
    return lastBlock;
}

/******************************************************************************/
/*                                 statements                                 */
/******************************************************************************/

/**
 * @brief take the last block, add it to a new If and add the new If to the
 * parent block.
 */
std::shared_ptr<Cnd> ProgramBuilder::createCnd(std::shared_ptr<Node> condition,
                                               std::shared_ptr<Block> block) {
    return std::make_shared<Cnd>(condition, block);
}

/**
 * @brief take the last block, add it to a new For and add the new For to the
 * parent block.
 */
std::shared_ptr<For> ProgramBuilder::createFor(Variable v,
                                               std::shared_ptr<Node> begin,
                                               std::shared_ptr<Node> end,
                                               std::shared_ptr<Node> step,
                                               std::shared_ptr<Block> block) {
    return std::make_shared<For>(v, begin, end, step, block);
}

/**
 * @brief take the last block, add it to a new While and add the new If to the
 * parent block.
 */
std::shared_ptr<Whl> ProgramBuilder::createWhl(std::shared_ptr<Node> condition,
                                               std::shared_ptr<Block> block) {
    return std::make_shared<Whl>(condition, block);
}

/******************************************************************************/
/*                                  funcalls                                  */
/******************************************************************************/

std::shared_ptr<FunctionCall>
ProgramBuilder::createFuncall(type_system::type type) {
    auto newFuncall = std::make_shared<FunctionCall>(
        funcallIds.back(), funcallParams.back(), type);
    funcallIds.pop_back();
    funcallParams.pop_back();
    return newFuncall;
}

void ProgramBuilder::newFuncall(std::string name) {
    funcallIds.push_back(name);
    funcallParams.push_back(std::list<std::shared_ptr<TypedNode>>());
}

void ProgramBuilder::createFunction(std::string name,
                                    std::shared_ptr<Block> operations,
                                    type_system::type returnType) {
    type_system::types argumentsTypes;
    for (Variable v : funParams) {
        argumentsTypes.push_back(v.type);
    }
    std::shared_ptr<Function> function = std::make_shared<Function>(
        name, funParams, operations,
        type_system::make_type<type_system::Function>(returnType, argumentsTypes));
    program->addFunction(function);
    funParams.clear();
}

void ProgramBuilder::pushFuncallParam(std::shared_ptr<TypedNode> newParam) {
    funcallParams.back().push_back(newParam);
}

/******************************************************************************/
/*                                 functions                                  */
/******************************************************************************/

void ProgramBuilder::pushFunctionParam(Variable newParam) {
    funParams.push_back(newParam);
}

/******************************************************************************/
/*                                  getters                                   */
/******************************************************************************/

std::shared_ptr<Program> ProgramBuilder::getProgram() const { return program; }

std::list<Variable> const &ProgramBuilder::getFunParams() const {
    return funParams;
}

type_system::types ProgramBuilder::getParamsTypes() const {
    type_system::types paramsTypes;
    for (Variable v : funParams) {
        paramsTypes.push_back(v.type);
        /* std::cout << "id => " << v.getId() << std::endl; */
    }
    return paramsTypes;
}
