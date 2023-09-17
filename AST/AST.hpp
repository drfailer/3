#ifndef __AST__
#define __AST__
#include "Types.hpp"
#include <cstdio>
#include <fstream>
#include <list>
#include <memory>
#include <string>

/*=============================================================================
# TODO - AST
- [ ] rewrite getters (const + refs)
- [ ] write a container class to group variable and arrays
- [ ] create a TypedElements abstract class to group elements with types

# TODO - Symtable
- [x] add array kind
- [x] add the utilities functions in bison file

# TODO - Parser
- [ ] add arrays to set (assignment)
- [ ] add arrays to the other rules
=============================================================================*/

/******************************************************************************/
/*                                    node                                    */
/******************************************************************************/

class ASTNode {
      public:
        virtual void compile(std::ofstream &, int) = 0;
        virtual void display() = 0;
        virtual ~ASTNode() = 0;
};

/******************************************************************************/
/*                                   block                                    */
/******************************************************************************/

/**
 * @brief  A `Block` refer to blocks of code between "{}". It contains the list
 *         of the instructions defined insite the brackets.
 */
class Block : public ASTNode {
      public:
        void compile(std::ofstream &, int = 0) override;
        void addOp(std::shared_ptr<ASTNode>);
        std::shared_ptr<ASTNode> getLastNode();
        void display() override;
        Block();

      private:
        std::list<std::shared_ptr<ASTNode>> operations;
};

/******************************************************************************/
/*                                  includes                                  */
/******************************************************************************/

/**
 * @brief  Include a file. Note: the language doesn't have a module system, so
 *         it acts a bit like a pre-processor instruction.
 *
 * TODO: remove, this is useless here.
 */
class Include : public ASTNode {
      public:
        std::string getLibName() const { return libName; }
        void display() override;
        void compile(std::ofstream &, int) override;
        Include(std::string);

      private:
        std::string libName;
};

/******************************************************************************/
/*                                  factors                                   */
/******************************************************************************/

class TypedElement : public ASTNode {
      public:
        virtual Type getType() const { return type; }
        void setType(Type type) { this->type = type; }
        ~TypedElement() = 0;

      protected:
        Type type;
};

/**
 * @brief  Basic values of an available type.
 */
class Value : public TypedElement {
      public:
        void compile(std::ofstream &, int) override;
        void display() override;
        type_t getValue() const;
        Value(type_t, Type);
        Value() = default;

      private:
        type_t value;
};

/**
 * @brief  Reference to a variable.
 *
 * NOTE: the type may be useless, it is here just in case.
 * TODO: justify or not the choice of having a type using the definition of the
 *       variable.
 */
class Variable : public TypedElement {
      public:
        void display() override;
        void compile(std::ofstream &, int) override;
        std::string getId() const;
        Variable(std::string, Type);

      private:
        std::string id;
};

class Array : public Variable {
      public:
        std::string getId() const;
        int getSize() const;
        Array(std::string, int, Type);
        ~Array() = default;

      protected:
        // TODO: this should be unsigned
        int size;
};

class ArrayDeclaration : public Array {
      public:
        void display() override;
        void compile(std::ofstream &, int) override;
        ArrayDeclaration(std::string, int, Type);
        ~ArrayDeclaration() = default;
};

class ArrayAccess : public Array {
      public:
        void display() override;
        void compile(std::ofstream &, int) override;
        std::shared_ptr<ASTNode> getIndex() const;
        ArrayAccess(std::string, int, Type, std::shared_ptr<ASTNode>);
        ~ArrayAccess() = default;

      private:
        std::shared_ptr<ASTNode> index;
};

/******************************************************************************/
/*                                 commands                                   */
/******************************************************************************/

/**
 * @brief  Represent an assignment operation. The "ifdef" check of the
 *         variable is done in the parser using the table of symbols.
 */
class Assignment : public ASTNode {
      public:
        std::shared_ptr<Variable> getVariable() const { return variable; }
        std::shared_ptr<TypedElement> getValue() const { return value; }
        void display() override;
        void compile(std::ofstream &, int) override;
        Assignment(std::shared_ptr<Variable>, std::shared_ptr<TypedElement>);

      private:
        std::shared_ptr<Variable> variable;
        std::shared_ptr<TypedElement> value;
};

/**
 * @brief  Declare a variable.
 *
 * NOTE: could be nice to have a type here.
 */
class Declaration : public ASTNode {
      public:
        void display() override;
        void compile(std::ofstream &, int) override;
        Declaration(Variable);

      private:
        Variable variable;
};

/******************************************************************************/
/*                                  funcall                                   */
/******************************************************************************/

/**
 * @brief  Calling a function. This function has a name and some parameters. The
 *         parameters are node as the can have multiple types (binary operation,
 *         variable, values, other funcall, ...)
 */
class Funcall : public TypedElement {
      public:
        std::string getFunctionName() const;
        void compile(std::ofstream &, int) override;
        std::list<std::shared_ptr<TypedElement>> getParams() const;
        Funcall(std::string, std::list<std::shared_ptr<TypedElement>>, Type);
        void display() override;

      private:
        std::string functionName;
        std::list<std::shared_ptr<TypedElement>> params;
};

// TODO: change this name
/******************************************************************************/
/*                                 statements                                 */
/******************************************************************************/

/**
 * @brief  A `Statement` is somthing that contains a `Block` (function
 *         definitions, if, for, while).
 */
class Statement : public ASTNode {
      public:
        Statement(std::shared_ptr<Block>);
        void display() override;

      protected:
        std::shared_ptr<Block> block;
};

/**
 * @brief  Fonction declaration. It has an id (function name), some parameters
 *         (which are just variable declarations) and a return type.
 *
 * TODO: create a return statment and manage return type.
 */
class Function : public Statement, public TypedElement {
      public:
        void display() override;
        void compile(std::ofstream &, int) override;
        Type getType() const override;
        Function(std::string, std::list<Variable>, std::shared_ptr<Block>,
                 std::list<Type>);

      private:
        std::string id;
        std::list<Variable> params;
        std::list<Type> type;
};

/**
 * @brief  If declaration. It has a condition which is a boolean operation (ref:
 *         parser).
 */
class If : public Statement {
      public:
        If(std::shared_ptr<ASTNode>, std::shared_ptr<Block>);
        void createElse(std::shared_ptr<Block>);
        void display() override;
        void compile(std::ofstream &, int) override;

      private:
        std::shared_ptr<ASTNode> condition; // TODO: put real conditions
        std::shared_ptr<Block> elseBlock;
};

/**
 * @brief  For declaration. The for loop has a "range" which has a begin, an end
 *         and a step value. It also has a loop variable.
 */
class For : public Statement {
      public:
        For(Variable, std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>,
            std::shared_ptr<ASTNode>, std::shared_ptr<Block>);
        void display() override;
        void compile(std::ofstream &, int) override;

      private:
        Variable var;
        std::shared_ptr<ASTNode> begin;
        std::shared_ptr<ASTNode> end;
        std::shared_ptr<ASTNode> step;
};

/**
 * @brief  While declaration. The while loop just has a condition.
 */
class While : public Statement {
      public:
        While(std::shared_ptr<ASTNode>, std::shared_ptr<Block>);
        void display() override;
        void compile(std::ofstream &, int) override;

      private:
        std::shared_ptr<ASTNode> condition; // TODO: create condition class
};

/******************************************************************************/
/*                           arithmetic operations                            */
/******************************************************************************/

class BinaryOperation : public ASTNode {
      public:
        BinaryOperation(std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>);
        void display() override;

      protected:
        std::shared_ptr<ASTNode> left;
        std::shared_ptr<ASTNode> right;
};

class AddOP : public BinaryOperation, public TypedElement {
      public:
        AddOP(std::shared_ptr<TypedElement>, std::shared_ptr<TypedElement>);
        void display() override; // +
        void compile(std::ofstream &, int) override;
};

class MnsOP : public BinaryOperation, public TypedElement {
      public:
        MnsOP(std::shared_ptr<TypedElement>, std::shared_ptr<TypedElement>);
        void display() override; // -
        void compile(std::ofstream &, int) override;
};

class TmsOP : public BinaryOperation, public TypedElement {
      public:
        TmsOP(std::shared_ptr<TypedElement>, std::shared_ptr<TypedElement>);
        void display() override; // *
        void compile(std::ofstream &, int) override;
};

class DivOP : public BinaryOperation, public TypedElement {
      public:
        DivOP(std::shared_ptr<TypedElement>, std::shared_ptr<TypedElement>);
        void display() override; // /
        void compile(std::ofstream &, int) override;
};

/******************************************************************************/
/*                             boolean operations                             */
/******************************************************************************/

class EqlOP : public BinaryOperation {
      public:
        EqlOP(std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>);
        void display() override; // left == right
        void compile(std::ofstream &, int) override;
};

class SupOP : public BinaryOperation {
      public:
        SupOP(std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>);
        void display() override; // left == right
        void compile(std::ofstream &, int) override;
};

class InfOP : public BinaryOperation {
      public:
        InfOP(std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>);
        void display() override; // left == right
        void compile(std::ofstream &, int) override;
};

class SeqOP : public BinaryOperation {
      public:
        SeqOP(std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>);
        void display() override; // left == right
        void compile(std::ofstream &, int) override;
};

class IeqOP : public BinaryOperation {
      public:
        IeqOP(std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>);
        void display() override; // left == right
        void compile(std::ofstream &, int) override;
};

class OrOP : public BinaryOperation {
      public:
        OrOP(std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>);
        void display() override; // left || right
        void compile(std::ofstream &, int) override;
};

class AndOP : public BinaryOperation {
      public:
        AndOP(std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>);
        void display() override; // left && right
        void compile(std::ofstream &, int) override;
};

class XorOP : public BinaryOperation {
      public:
        XorOP(std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>);
        void display() override; // left ^ right
        void compile(std::ofstream &, int) override;
};

class NotOP : public ASTNode {
      public:
        NotOP(std::shared_ptr<ASTNode>);
        void display() override; // !param
        void compile(std::ofstream &, int) override;

      private:
        std::shared_ptr<ASTNode> param;
};

/******************************************************************************/
/*                                     IO                                     */
/******************************************************************************/

/**
 * @brief  Gestion de l'affichage. On ne peut afficher que soit une chaine de
 *         caratères (`"str"`), soit une variable.
 */
class Print : public ASTNode {
      public:
        void display() override;
        Print(std::string);
        Print(std::shared_ptr<ASTNode>);
        void compile(std::ofstream &, int) override;

      private:
        std::string str;
        std::shared_ptr<ASTNode> content;
};

/**
 * @brief  Gestion de la saisie clavier.
 */
class Read : public ASTNode {
      public:
        void display() override;
        Read(std::shared_ptr<TypedElement>);
        void compile(std::ofstream &, int) override;

      private:
        std::shared_ptr<TypedElement> variable;
};

/******************************************************************************/
/*                                   return                                   */
/******************************************************************************/

class Return : public ASTNode {
      public:
        void display() override;
        void compile(std::ofstream &, int) override;
        Return(std::shared_ptr<ASTNode>);

      private:
        std::shared_ptr<ASTNode> returnExpr;
};

#endif
