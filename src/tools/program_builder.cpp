#include "program_builder.hpp"

ProgramBuilder::ProgramBuilder() : program_(std::make_shared<Program>()) {}

void ProgramBuilder::display() { program_->display(); }

/******************************************************************************/
/*                                   blocks                                   */
/******************************************************************************/

/**
 * @brief  Add `command` to the last block of the block stack.
 */
void ProgramBuilder::pushBlock(std::shared_ptr<Node> command) {
    blocks.top()->add(command);
}

/**
 * @brief  create an empty block on the top of the blocks stack
 */
void ProgramBuilder::beginBlock() {
    blocks.push(std::make_shared<Block>());
}

/**
 * @brief  pop the last block of the blocks stack
 */
std::shared_ptr<Block> ProgramBuilder::endBlock() {
    std::shared_ptr<Block> lastBlock = blocks.top();
    blocks.pop();
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
ProgramBuilder::createFuncall(type_system::type_t type) {
    auto newFuncall = std::make_shared<FunctionCall>(
        funcallIds_.back(), functionCallsParameters_.back(), type);
    funcallIds_.pop_back();
    functionCallsParameters_.pop_back();
    return newFuncall;
}

void ProgramBuilder::newFuncall(std::string name) {
    funcallIds_.push_back(name);
    functionCallsParameters_.push_back(std::list<std::shared_ptr<TypedNode>>());
}

void ProgramBuilder::createFunction(std::string name,
                                    std::shared_ptr<Block> operations,
                                    type_system::type_t returnType) {
    type_system::types_t argumentsTypes;
    for (Variable v : currFunctionParameters_) {
        argumentsTypes.push_back(v.type);
    }
    std::shared_ptr<Function> function = std::make_shared<Function>(
        name, currFunctionParameters_, operations,
        type_system::make_type<type_system::Function>(returnType, argumentsTypes));
    program_->addFunction(function);
    currFunctionParameters_.clear();
}

void ProgramBuilder::pushFuncallParam(std::shared_ptr<TypedNode> newParam) {
    functionCallsParameters_.back().push_back(newParam);
}

/******************************************************************************/
/*                                 functions                                  */
/******************************************************************************/

void ProgramBuilder::pushFunctionParam(Variable newParam) {
    currFunctionParameters_.push_back(newParam);
}

/******************************************************************************/
/*                                  getters                                   */
/******************************************************************************/

std::shared_ptr<Program> ProgramBuilder::program() const { return program_; }

std::list<Variable> const &ProgramBuilder::functionParameters() const {
    return currFunctionParameters_;
}

type_system::types_t ProgramBuilder::parametersTypes() const {
    type_system::types_t paramsTypes;
    for (Variable v : currFunctionParameters_) {
        paramsTypes.push_back(v.type);
        /* std::cout << "id => " << v.getId() << std::endl; */
    }
    return paramsTypes;
}
