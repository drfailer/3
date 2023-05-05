#include "ProgramBuilder.hpp"

ProgramBuilder::ProgramBuilder()
{
  program = std::make_shared<Program>();
  blocks = std::list<std::shared_ptr<Block>>();
  funParams = std::list<Variable>();
  funcallParams = std::list<std::list<std::shared_ptr<TypedElement>>>();
}

void ProgramBuilder::display()
{
  program->display();
}

void ProgramBuilder::addInclude(std::string i)
{
  program->addInclude(i);
}

/******************************************************************************/
/*                                   blocks                                   */
/******************************************************************************/

/**
 * @brief  Add `command` to the last block of the block stack.
 */
void ProgramBuilder::pushBlock(std::shared_ptr<ASTNode> command)
{
  blocks.back()->addOp(command);
}

/**
 * @brief  create an empty block on the top of the blocks stack
 */
void ProgramBuilder::beginBlock()
{
  blocks.push_back(std::make_shared<Block>());
}

/**
 * @brief  pop the last block of the blocks stack
 */
std::shared_ptr<Block> ProgramBuilder::endBlock()
{
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
std::shared_ptr<If> ProgramBuilder::createIf(std::shared_ptr<ASTNode> condition,
    std::shared_ptr<Block> block)
{
  return std::make_shared<If>(condition, block);
}

/**
 * @brief take the last block, add it to a new For and add the new For to the
 * parent block.
 */
std::shared_ptr<For> ProgramBuilder::createFor(Variable v,
    std::shared_ptr<ASTNode> begin, std::shared_ptr<ASTNode> end,
    std::shared_ptr<ASTNode> step, std::shared_ptr<Block> block)
{
  return std::make_shared<For>(v, begin, end, step, block);
}

/**
 * @brief take the last block, add it to a new While and add the new If to the
 * parent block.
 */
std::shared_ptr<While> ProgramBuilder::createWhile(std::shared_ptr<ASTNode> condition,
    std::shared_ptr<Block> block)
{
  return std::make_shared<While>(condition, block);
}


/******************************************************************************/
/*                                  funcalls                                  */
/******************************************************************************/

std::shared_ptr<Funcall> ProgramBuilder::createFuncall()
{
  std::shared_ptr<Funcall> newFuncall =
    std::make_shared<Funcall>(funcallIds.back(), funcallParams.back(), VOID);
  funcallIds.pop_back();
  funcallParams.pop_back();
  return newFuncall;
}

void ProgramBuilder::newFuncall(std::string name)
{
  funcallIds.push_back(name);
  funcallParams.push_back(std::list<std::shared_ptr<TypedElement>>());
}

void ProgramBuilder::createFunction(std::string name, std::shared_ptr<Block>
    operations, Type returnType)
{
  std::list<Type> type;
  for (Variable v : funParams) {
    type.push_back(v.getType());
  }
  type.push_back(returnType);
  std::shared_ptr<Function> newfun =
    std::make_shared<Function>(name, funParams, operations, type);
  program->addFunction(newfun);
  funParams.clear();
}

void ProgramBuilder::pushFuncallParam(std::shared_ptr<TypedElement> newParam)
{
  funcallParams.back().push_back(newParam);
}

/******************************************************************************/
/*                                 functions                                  */
/******************************************************************************/

void ProgramBuilder::pushFunctionParam(Variable newParam)
{
  funParams.push_back(newParam);
}

/******************************************************************************/
/*                                  getters                                   */
/******************************************************************************/

std::shared_ptr<Program> ProgramBuilder::getProgram() const
{
  return program;
}

std::list<Variable> ProgramBuilder::getFunParams() const
{
  return funParams;
}

std::list<Type> ProgramBuilder::getParamsTypes() const
{
  std::list<Type> paramsTypes;
  for (Variable v : funParams) {
    paramsTypes.push_back(v.getType());
    /* std::cout << "id => " << v.getId() << std::endl; */
  }
  return paramsTypes;
}

