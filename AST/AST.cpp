#include "AST.hpp"
#include "Types.hpp"
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <cstring>

void indent(std::ofstream &fs, int lvl) {
        for (int i = 0; i < lvl; ++i) {
                fs << '\t';
        }
}

/* -------------------------------------------------------------------------- */

ASTNode::~ASTNode() {}

TypedElement::~TypedElement() {}

/* -------------------------------------------------------------------------- */

Value::Value(type_t value, Type type) : value(value) {
        TypedElement::type = type;
}

type_t Value::getValue() const { return value; }

void Value::display() {
        switch (type) {
        case INT:
                std::cout << value.i;
                break;
        case FLT:
                std::cout << value.f;
                break;
        case CHR:
                std::cout << "'" << value.c << "'";
                break;
        default:
                break;
        }
}

void Value::compile(std::ofstream &fs, int lvl) {
        switch (type) {
        case INT:
                fs << value.i;
                break;
        case FLT:
                fs << value.f;
                break;
        case CHR:
                fs << "'" << value.c << "'";
                break;
        case ARR_CHR: {
                // WARN: the '"' are in the string (this may change).
                // TODO: this doesn't work, the value is technically correct but
                // it doesn't take in count the size of the targeted array.
                std::string str = value.s;
                fs << "[c for c in " << str << "]+[0]";
        } break;
        default:
                break;
        }
}

/* -------------------------------------------------------------------------- */

Variable::Variable(std::string id, Type type) : id(id) {
        TypedElement::type = type;
}

std::string Variable::getId() const { return id; }

void Variable::display() { std::cout << id; }

void Variable::compile(std::ofstream &fs, int lvl) { fs << id; }

/* -------------------------------------------------------------------------- */

Array::Array(std::string name, int size, Type type)
    : Variable(name, type), size(size) {}

std::string Array::getId() const { return Variable::getId(); }

int Array::getSize() const { return size; }

ArrayDeclaration::ArrayDeclaration(std::string name, int size, Type type)
    : Array(name, size, type) {}

void ArrayDeclaration::display() {
        std::cout << Array::getId() << "[" << size << "]";
}

void ArrayDeclaration::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        fs << getId() << "=[0 for _ in range(" << size << ")]";
}

ArrayAccess::ArrayAccess(std::string name, Type type,
                         std::shared_ptr<ASTNode> index)
    : Array(name, -1, type), index(index) {}

std::shared_ptr<ASTNode> ArrayAccess::getIndex() const { return index; }

void ArrayAccess::display() {
        std::cout << Variable::getId() << "[";
        index->display();
        std::cout << "]";
}

void ArrayAccess::compile(std::ofstream &fs, int lvl) {
        fs << Variable::getId() << "[";
        index->compile(fs, 0);
        fs << "]";
}

/* -------------------------------------------------------------------------- */

Type Function::getType() const { return type.back(); }

void Function::display() {
        std::cout << "Function(" << id << ", [";
        for (Variable p : params) {
                p.display();
                std::cout << ", ";
        }
        std::cout << "], ";
        Statement::display();
        std::cout << ")" << std::endl;
}

Function::Function(std::string id, std::list<Variable> params,
                   std::shared_ptr<Block> instructions, std::list<Type> type)
    : Statement(instructions), id(id), params(params), type(type) {}

void Function::compile(std::ofstream &fs, int lvl) {
        fs << "def " << id << "(";
        if (params.size() > 0) {
                std::list<Variable> tmp = params;
                tmp.front().compile(fs, 0);
                tmp.pop_front();
                for (Variable v : tmp) {
                        fs << ",";
                        v.compile(fs, 0);
                }
        }
        fs << "):" << std::endl;
        block->compile(fs, 0);
}

/* -------------------------------------------------------------------------- */

Include::Include(std::string name) : libName(name) {}

void Include::display() {
        std::cout << "Include(" << libName << ")" << std::endl;
}

void Include::compile(std::ofstream &, int) {}

/* -------------------------------------------------------------------------- */

Block::Block() { operations = std::list<std::shared_ptr<ASTNode>>(); }

void Block::addOp(std::shared_ptr<ASTNode> operation) {
        operations.push_back(operation);
}

std::shared_ptr<ASTNode> Block::getLastNode() { return operations.back(); }

void Block::display() {
        std::cout << "Block(" << std::endl;
        for (std::shared_ptr<ASTNode> o : operations) {
                o->display();
        }
        std::cout << ")" << std::endl;
}

void Block::compile(std::ofstream &fs, int lvl) {
        for (std::shared_ptr<ASTNode> op : operations) {
                op->compile(fs, lvl + 1);
                fs << std::endl;
        }
}

/* -------------------------------------------------------------------------- */

Assignment::Assignment(std::shared_ptr<Variable> variable,
                         std::shared_ptr<TypedElement> value)
    : variable(variable), value(value) {}

void Assignment::display() {
        std::cout << "Assignment(";
        variable->display();
        std::cout << ",";
        value->display();
        std::cout << ")" << std::endl;
}

void Assignment::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        // TODO: find a better way to handle this case
        if (variable->getType() == ARR_CHR && value->getType() == ARR_CHR) {
                std::shared_ptr<Array> array = std::dynamic_pointer_cast<Array>(variable);
                std::shared_ptr<Value> val = std::dynamic_pointer_cast<Value>(value);
                // WARN: the value contains the '"'
                std::string str = val->getValue().s;
                unsigned int size = std::min(array->getSize(), (int) str.size() - 2 + 1);

                // reset the array before assignment of the string
                fs << array->getId() << "=[0 for _ in range(" << array->getSize() << ")]" << std::endl;
                indent(fs, lvl);
                fs << "for _ZZ_TRANSPILER_STRINGSET_INDEX in range(" << size - 1 << "):" << std::endl;
                indent(fs, lvl + 1);
                fs << variable->getId() << "[_ZZ_TRANSPILER_STRINGSET_INDEX]=";
                fs << str << "[_ZZ_TRANSPILER_STRINGSET_INDEX]";
        } else {
                variable->compile(fs, lvl);
                fs << "=";
                switch (variable->getType()) {
                case INT:
                        fs << "int(";
                        break;
                case CHR:
                        fs << "chr(";
                        break;
                case FLT:
                        fs << "float(";
                        break;
                default:
                        fs << "(";
                        break;
                }
                value->compile(fs, lvl);
                fs << ")";
        }
}

/* -------------------------------------------------------------------------- */

Declaration::Declaration(Variable variable) : variable(variable) {}

void Declaration::display() {
        std::cout << "Declaration(";
        variable.display();
        std::cout << ")" << std::endl;
}

void Declaration::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        fs << "# " << variable.getType() << " "
           << variable.getId();
}

/* -------------------------------------------------------------------------- */

Funcall::Funcall(std::string functionName,
                 std::list<std::shared_ptr<TypedElement>> params, Type type)
    : functionName(functionName), params(params) {
        TypedElement::type = type;
}

void Funcall::display() {
        std::cout << "Funcall(" << functionName << ", [";
        for (std::shared_ptr<ASTNode> p : params) {
                p->display();
                std::cout << ", ";
        }
        std::cout << "])" << std::endl;
}

std::list<std::shared_ptr<TypedElement>> Funcall::getParams() const {
        return params;
}

std::string Funcall::getFunctionName() const { return functionName; }

void Funcall::compile(std::ofstream &fs, int lvl) {
        // TODO: there is more work to do when we pas a string to the function
        indent(fs, lvl);
        fs << functionName << "(";
        for (std::shared_ptr<ASTNode> p : params) {
                p->compile(fs, 0);
                if (p != params.back())
                        fs << ',';
        }
        fs << ")";
}

/******************************************************************************/
/*                                 statements                                 */
/******************************************************************************/

Statement::Statement(std::shared_ptr<Block> b) : block(b) {}

void Statement::display() { block->display(); }

/* -------------------------------------------------------------------------- */

If::If(std::shared_ptr<ASTNode> c, std::shared_ptr<Block> b)
    : Statement(b), condition(c), elseBlock(nullptr) {}

void If::createElse(std::shared_ptr<Block> block) { elseBlock = block; }

void If::display() {
        std::cout << "If(";
        condition->display();
        std::cout << ", ";
        Statement::display();
        if (elseBlock != nullptr) { // print else block if needed
                std::cout << ", Else(";
                elseBlock->display();
                std::cout << ")" << std::endl;
        }
        std::cout << ")" << std::endl;
}

void If::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        fs << "if ";
        condition->compile(fs, 0);
        fs << ":" << std::endl;
        block->compile(fs, lvl);
        if (elseBlock != nullptr) {
                indent(fs, lvl);
                fs << "else:" << std::endl;
                elseBlock->compile(fs, lvl);
        }
}

/* -------------------------------------------------------------------------- */

For::For(Variable v, std::shared_ptr<ASTNode> begin,
         std::shared_ptr<ASTNode> end, std::shared_ptr<ASTNode> step,
         std::shared_ptr<Block> b)
    : Statement(b), begin(begin), end(end), step(step), var(v) {}

void For::display() {
        std::cout << "For(";
        var.display();
        std::cout << ", range(";
        begin->display();
        std::cout << ",";
        end->display();
        std::cout << ",";
        step->display();
        std::cout << "), ";
        Statement::display();
        std::cout << ")" << std::endl;
}

void For::compile(std::ofstream &fs, int lvl) {
        // TODO: vérifier les type et cast si besoin
        indent(fs, lvl);
        fs << "for ";
        var.compile(fs, 0);
        fs << " in range(";
        begin->compile(fs, 0);
        fs << ",";
        end->compile(fs, 0);
        fs << ",";
        step->compile(fs, 0);
        fs << "):" << std::endl;
        block->compile(fs, lvl);
}

/* -------------------------------------------------------------------------- */

While::While(std::shared_ptr<ASTNode> c, std::shared_ptr<Block> b)
    : Statement(b), condition(c) {}

void While::display() {
        std::cout << "While(";
        condition->display();
        std::cout << ", ";
        Statement::display();
        std::cout << ")" << std::endl;
}

void While::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        fs << "while ";
        condition->compile(fs, 0);
        fs << ":" << std::endl;
        block->compile(fs, lvl);
}

/******************************************************************************/
/*                                 operators                                  */
/******************************************************************************/

BinaryOperation::BinaryOperation(std::shared_ptr<ASTNode> left,
                                 std::shared_ptr<ASTNode> right)
    : left(left), right(right) {}

/******************************************************************************/
/*                           arithemtic operations                            */
/******************************************************************************/

Type selectType(Type left, Type right) {
        Type type;
        if (left == INT && right == INT) {
                type = INT;
        } else {
                type = FLT;
        }
        return type;
}

void BinaryOperation::display() {
        left->display();
        std::cout << ", ";
        right->display();
        std::cout << ")";
}

AddOP::AddOP(std::shared_ptr<TypedElement> left,
             std::shared_ptr<TypedElement> right)
    : BinaryOperation(left, right) {
        type = selectType(left->getType(), right->getType());
}

void AddOP::display() {
        std::cout << "AddOP(";
        BinaryOperation::display();
}

void AddOP::compile(std::ofstream &fs, int lvl) {
        fs << "(";
        left->compile(fs, 0);
        fs << "+";
        right->compile(fs, 0);
        fs << ")";
}

MnsOP::MnsOP(std::shared_ptr<TypedElement> left,
             std::shared_ptr<TypedElement> right)
    : BinaryOperation(left, right) {
        type = selectType(left->getType(), right->getType());
}

void MnsOP::display() {
        std::cout << "MnsOP(";
        BinaryOperation::display();
}

void MnsOP::compile(std::ofstream &fs, int lvl) {
        fs << "(";
        left->compile(fs, 0);
        fs << "-";
        right->compile(fs, 0);
        fs << ")";
}

TmsOP::TmsOP(std::shared_ptr<TypedElement> left,
             std::shared_ptr<TypedElement> right)
    : BinaryOperation(left, right) {
        type = selectType(left->getType(), right->getType());
}

void TmsOP::display() {
        std::cout << "TmsOP(";
        BinaryOperation::display();
}

void TmsOP::compile(std::ofstream &fs, int lvl) {
        fs << "(";
        left->compile(fs, 0);
        fs << "*";
        right->compile(fs, 0);
        fs << ")";
}

DivOP::DivOP(std::shared_ptr<TypedElement> left,
             std::shared_ptr<TypedElement> right)
    : BinaryOperation(left, right) {
        type = selectType(left->getType(), right->getType());
}

void DivOP::display() {
        std::cout << "DivOP(";
        BinaryOperation::display();
}

void DivOP::compile(std::ofstream &fs, int lvl) {
        fs << "(";
        left->compile(fs, 0);
        fs << "/";
        right->compile(fs, 0);
        fs << ")";
}

/******************************************************************************/
/*                             boolean operations                             */
/******************************************************************************/

EqlOP::EqlOP(std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right)
    : BinaryOperation(left, right) {}

void EqlOP::display() {
        std::cout << "EqlOP(";
        BinaryOperation::display();
}

void EqlOP::compile(std::ofstream &fs, int lvl) {
        left->compile(fs, 0);
        fs << "==";
        right->compile(fs, 0);
}

SupOP::SupOP(std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right)
    : BinaryOperation(left, right) {}

void SupOP::display() {
        std::cout << "SupOP(";
        BinaryOperation::display();
}

void SupOP::compile(std::ofstream &fs, int lvl) {
        left->compile(fs, 0);
        fs << ">";
        right->compile(fs, 0);
}

InfOP::InfOP(std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right)
    : BinaryOperation(left, right) {}

void InfOP::display() {
        std::cout << "InfOP(";
        BinaryOperation::display();
}

void InfOP::compile(std::ofstream &fs, int lvl) {
        left->compile(fs, 0);
        fs << "<";
        right->compile(fs, 0);
}

SeqOP::SeqOP(std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right)
    : BinaryOperation(left, right) {}

void SeqOP::display() {
        std::cout << "SeqOP(";
        BinaryOperation::display();
}

void SeqOP::compile(std::ofstream &fs, int lvl) {
        left->compile(fs, 0);
        fs << ">=";
        right->compile(fs, 0);
}

IeqOP::IeqOP(std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right)
    : BinaryOperation(left, right) {}

void IeqOP::display() {
        std::cout << "IeqOP(";
        BinaryOperation::display();
}

void IeqOP::compile(std::ofstream &fs, int lvl) {
        left->compile(fs, 0);
        fs << "<=";
        right->compile(fs, 0);
}

OrOP::OrOP(std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right)
    : BinaryOperation(left, right) {}

void OrOP::display() {
        std::cout << "OrOP(";
        BinaryOperation::display();
}

void OrOP::compile(std::ofstream &fs, int lvl) {
        left->compile(fs, 0);
        fs << " or ";
        right->compile(fs, 0);
}

AndOP::AndOP(std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right)
    : BinaryOperation(left, right) {}

void AndOP::display() {
        std::cout << "AndOP(";
        BinaryOperation::display();
}

void AndOP::compile(std::ofstream &fs, int lvl) {
        left->compile(fs, 0);
        fs << " and ";
        right->compile(fs, 0);
}

XorOP::XorOP(std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right)
    : BinaryOperation(left, right) {}

void XorOP::display() {
        std::cout << "XorOP(";
        BinaryOperation::display();
}

void XorOP::compile(std::ofstream &fs, int lvl) {
        left->compile(fs, 0); // TODO
        fs << " and ";
        right->compile(fs, 0);
}

NotOP::NotOP(std::shared_ptr<ASTNode> param) : param(param) {}

void NotOP::display() {
        std::cout << "NotOP(";
        param->display();
        std::cout << ")";
}

void NotOP::compile(std::ofstream &fs, int lvl) {
        fs << "not(";
        param->compile(fs, 0);
        fs << ") ";
}

/******************************************************************************/
/*                                     IO                                     */
/******************************************************************************/

Print::Print(std::shared_ptr<ASTNode> content) : str(""), content(content) {}

Print::Print(std::string str)
    : str(str), content(std::shared_ptr<ASTNode>(nullptr)) {}

void Print::display() {
        std::cout << "Print(";
        if (content != nullptr) {
                content->display();
        } else {
                std::cout << str;
        }
        std::cout << ");" << std::endl;
}

void Print::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        fs << "print(";
        if (content == nullptr) {
                fs << str;
        } else {
                content->compile(fs, 0);
        }
        fs << ",end=\"\")";
}

Read::Read(std::shared_ptr<TypedElement> variable) : variable(variable) {}

void Read::display() {
        std::cout << "Read(";
        variable->display();
        std::cout << ")" << std::endl;
}

void Read::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        variable->compile(fs, 0);
        switch (variable->getType()) {
        case INT:
                fs << " = int(input())";
                break;
        case FLT:
                fs << " = flt(input())";
                break;
        default:
                fs << " = input()";
                break;
        }
}

/******************************************************************************/
/*                                   return                                   */
/******************************************************************************/

Return::Return(std::shared_ptr<ASTNode> returnExpr) : returnExpr(returnExpr) {}

void Return::display() {
        std::cout << "Return(";
        returnExpr->display();
        std::cout << ")";
}

void Return::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        fs << "return ";
        returnExpr->compile(fs, 0);
}
