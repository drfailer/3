#include "program_builder.hpp"

void ProgramBuilder::display() {
    for (auto node : program) {
        node::print_node(node);
    }
}

/******************************************************************************/
/*                                   blocks                                   */
/******************************************************************************/

/**
 * @brief  Add `command` to the last block of the block stack.
 */
void ProgramBuilder::pushBlock(node::Node *command) {
    blocks.top()->nodes.push_back(command);
}

/**
 * @brief  create an empty block on the top of the blocks stack
 */
void ProgramBuilder::beginBlock() {
    // TODO: use a pool
    blocks.push(new node::Block());
}

/**
 * @brief  pop the last block of the blocks stack
 */
node::Block *ProgramBuilder::endBlock() {
    node::Block *lastBlock = blocks.top();
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
node::CndStmt *ProgramBuilder::createCnd(node::Node *condition,
                                         node::Block *block) {
    return new node::CndStmt{condition, block};
}

/**
 * @brief take the last block, add it to a new For and add the new For to the
 * parent block.
 */
node::ForStmt *ProgramBuilder::createFor(std::string const index_id,
                                         node::Node *start, node::Node *end,
                                         node::Node *step, node::Block *block) {
    return new node::ForStmt{index_id, start, end, step, block};
}

/**
 * @brief take the last block, add it to a new While and add the new If to the
 * parent block.
 */
node::WhlStmt *ProgramBuilder::createWhl(node::Node *condition,
                                         node::Block *block) {
    return new node::WhlStmt{condition, block};
}

/******************************************************************************/
/*                                  funcalls                                  */
/******************************************************************************/

node::Node *ProgramBuilder::createFuncall() {
    auto args = functionCallsParameters.back();
    auto funcall = node::create_function_call({}, funcallIds.back(), std::move(args));
    funcallIds.pop_back();
    functionCallsParameters.pop_back();
    return funcall;
}

void ProgramBuilder::newFuncall(std::string const &name) {
    funcallIds.push_back(name);
    functionCallsParameters.push_back({});
}

void ProgramBuilder::createFunction(std::string const &name,
                                    node::Block *block) {
    program.push_back(node::create_function_definition(
        {}, name, std::move(currFunctionParameters), block));
    currFunctionParameters = std::list<std::string>();
}

void ProgramBuilder::pushFuncallParam(node::Node *arg) {
    functionCallsParameters.back().push_back(arg);
}

/******************************************************************************/
/*                                 functions                                  */
/******************************************************************************/

void ProgramBuilder::pushFunctionParam(std::string const &arg) {
    currFunctionParameters.push_back(arg);
}
