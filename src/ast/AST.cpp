#include "AST.hpp"
#include "Types.hpp"
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <cstring>

void indent(std::ofstream &fs, int lvl) {
        for (int i = 0; i < lvl; ++i) {
                fs << '\t';
        }
}

/* -------------------------------------------------------------------------- */

Node::~Node() {}

TypedNode::~TypedNode() {}

/* -------------------------------------------------------------------------- */

void Value::display() {
        switch (type_) {
        case INT:
                std::cout << value_._int;
                break;
        case FLT:
                std::cout << value_._flt;
                break;
        case CHR:
                std::cout << "'" << value_._chr << "'";
                break;
        default:
                break;
        }
}

void Value::compile(std::ofstream &fs, int) {
        switch (type_) {
        case INT:
                fs << value_._int;
                break;
        case FLT:
                fs << value_._flt;
                break;
        case CHR:
                fs << "'" << value_._chr << "'";
                break;
        case ARR_CHR: {
                // WARN: the '"' are in the string (this may change).
                // TODO: this doesn't work, the value is technically correct but
                // it doesn't take in count the size of the targeted array.
                std::string str = value_._str;
                fs << "[c for c in " << str << "]+[0]";
        } break;
        default:
                break;
        }
}

/* -------------------------------------------------------------------------- */

void Variable::display() { std::cout << id_; }

void Variable::compile(std::ofstream &fs, int) { fs << id_; }

/* -------------------------------------------------------------------------- */

void ArrayDeclaration::display() {
        std::cout << this->id() << "[" << size_ << "]";
}

void ArrayDeclaration::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        fs << this->id() << "=[0 for _ in range(" << size_ << ")]";
}

void ArrayAccess::display() {
        std::cout << Variable::id() << "[";
        index_->display();
        std::cout << "]";
}

void ArrayAccess::compile(std::ofstream &fs, int) {
        fs << Variable::id() << "[";
        index_->compile(fs, 0);
        fs << "]";
}

/* -------------------------------------------------------------------------- */


void Function::display() {
        std::cout << "Function(" << id_ << ", [";
        for (Variable p : parameters_) {
                p.display();
                std::cout << ", ";
        }
        std::cout << "], ";
        block_->display();
        std::cout << ")" << std::endl;
}

void Function::compile(std::ofstream &fs, int) {
        fs << "def " << id_ << "(";
        if (parameters_.size() > 0) {
                std::list<Variable> tmp = parameters_;
                tmp.front().compile(fs, 0);
                tmp.pop_front();
                for (Variable v : tmp) {
                        fs << ",";
                        v.compile(fs, 0);
                }
        }
        fs << "):" << std::endl;
        block_->compile(fs, 0);
}

/* -------------------------------------------------------------------------- */


void Block::display() {
        std::cout << "Block(" << std::endl;
        for (std::shared_ptr<Node> o : instructions_) {
                o->display();
        }
        std::cout << ")" << std::endl;
}

void Block::compile(std::ofstream &fs, int lvl) {
        for (std::shared_ptr<Node> op : instructions_) {
                op->compile(fs, lvl + 1);
                fs << std::endl;
        }
}

/* -------------------------------------------------------------------------- */

void Assignment::display() {
        std::cout << "Assignment(";
        variable_->display();
        std::cout << ",";
        value_->display();
        std::cout << ")" << std::endl;
}

void Assignment::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        // TODO: find a better way to handle this case
        if (variable_->type() == ARR_CHR && value_->type() == ARR_CHR) {
                std::shared_ptr<Array> array = std::dynamic_pointer_cast<Array>(variable_);
                std::shared_ptr<Value> val = std::dynamic_pointer_cast<Value>(value_);
                // WARN: the value contains the '"'
                std::string str = val->value()._str;
                // TODO: this should be done at runtime !
                unsigned int size = std::min(array->size(), (int) str.size() - 2 + 1);

                // reset the array before assignment of the string
                fs << array->id() << "=[0 for _ in range(" << array->size() << ")]" << std::endl;
                indent(fs, lvl);
                fs << "for _ZZ_TRANSPILER_STRINGSET_INDEX in range(" << size - 1 << "):" << std::endl;
                indent(fs, lvl + 1);
                fs << variable_->id() << "[_ZZ_TRANSPILER_STRINGSET_INDEX]=";
                fs << str << "[_ZZ_TRANSPILER_STRINGSET_INDEX]";
        } else {
                variable_->compile(fs, lvl);
                fs << "=";
                switch (variable_->type()) {
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
                value_->compile(fs, lvl);
                fs << ")";
        }
}

/* -------------------------------------------------------------------------- */


void Declaration::display() {
        std::cout << "Declaration(";
        variable_.display();
        std::cout << ")" << std::endl;
}

void Declaration::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        fs << "# " << variable_.type() << " "
           << variable_.id();
}

/* -------------------------------------------------------------------------- */

void Funcall::display() {
        std::cout << "Funcall(" << functionName_ << ", [";
        for (std::shared_ptr<Node> p : params_) {
                p->display();
                std::cout << ", ";
        }
        std::cout << "])" << std::endl;
}

void Funcall::compile(std::ofstream &fs, int lvl) {
        // TODO: there is more work to do when we pas a string to the function
        indent(fs, lvl);
        fs << functionName_ << "(";
        for (std::shared_ptr<Node> p : params_) {
                p->compile(fs, 0);
                if (p != params_.back())
                        fs << ',';
        }
        fs << ")";
}

/******************************************************************************/
/*                                 statements                                 */
/******************************************************************************/

/* -------------------------------------------------------------------------- */

void If::display() {
        std::cout << "If(";
        condition_->display();
        std::cout << ", ";
        block_->display();
        if (elseBlock_ != nullptr) { // print else block if needed
                std::cout << ", Else(";
                elseBlock_->display();
                std::cout << ")" << std::endl;
        }
        std::cout << ")" << std::endl;
}

void If::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        fs << "if ";
        condition_->compile(fs, 0);
        fs << ":" << std::endl;
        block_->compile(fs, lvl);
        if (elseBlock_ != nullptr) {
                indent(fs, lvl);
                fs << "else:" << std::endl;
                elseBlock_->compile(fs, lvl);
        }
}

/* -------------------------------------------------------------------------- */

void For::display() {
        std::cout << "For(";
        variable_.display();
        std::cout << ", range(";
        begin_->display();
        std::cout << ",";
        end_->display();
        std::cout << ",";
        step_->display();
        std::cout << "), ";
        block_->display();
        std::cout << ")" << std::endl;
}

void For::compile(std::ofstream &fs, int lvl) {
        // TODO: vérifier les type et cast si besoin
        indent(fs, lvl);
        fs << "for ";
        variable_.compile(fs, 0);
        fs << " in range(";
        begin_->compile(fs, 0);
        fs << ",";
        end_->compile(fs, 0);
        fs << ",";
        step_->compile(fs, 0);
        fs << "):" << std::endl;
        block_->compile(fs, lvl);
}

/* -------------------------------------------------------------------------- */

void While::display() {
        std::cout << "While(";
        condition_->display();
        std::cout << ", ";
        block_->display();
        std::cout << ")" << std::endl;
}

void While::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        fs << "while ";
        condition_->compile(fs, 0);
        fs << ":" << std::endl;
        block_->compile(fs, lvl);
}

/******************************************************************************/
/*                                 operators                                  */
/******************************************************************************/

BinaryOperation::BinaryOperation(std::shared_ptr<Node> left,
                                 std::shared_ptr<Node> right)
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

AddOP::AddOP(std::shared_ptr<TypedNode> left,
             std::shared_ptr<TypedNode> right)
    : BinaryOperation(left, right) {
        type_ = selectType(left->type(), right->type());
}

void AddOP::display() {
        std::cout << "AddOP(";
        BinaryOperation::display();
}

void AddOP::compile(std::ofstream &fs, int) {
        fs << "(";
        left->compile(fs, 0);
        fs << "+";
        right->compile(fs, 0);
        fs << ")";
}

MnsOP::MnsOP(std::shared_ptr<TypedNode> left,
             std::shared_ptr<TypedNode> right)
    : BinaryOperation(left, right) {
        type_ = selectType(left->type(), right->type());
}

void MnsOP::display() {
        std::cout << "MnsOP(";
        BinaryOperation::display();
}

void MnsOP::compile(std::ofstream &fs, int) {
        fs << "(";
        left->compile(fs, 0);
        fs << "-";
        right->compile(fs, 0);
        fs << ")";
}

TmsOP::TmsOP(std::shared_ptr<TypedNode> left,
             std::shared_ptr<TypedNode> right)
    : BinaryOperation(left, right) {
        type_ = selectType(left->type(), right->type());
}

void TmsOP::display() {
        std::cout << "TmsOP(";
        BinaryOperation::display();
}

void TmsOP::compile(std::ofstream &fs, int) {
        fs << "(";
        left->compile(fs, 0);
        fs << "*";
        right->compile(fs, 0);
        fs << ")";
}

DivOP::DivOP(std::shared_ptr<TypedNode> left,
             std::shared_ptr<TypedNode> right)
    : BinaryOperation(left, right) {
        type_ = selectType(left->type(), right->type());
}

void DivOP::display() {
        std::cout << "DivOP(";
        BinaryOperation::display();
}

void DivOP::compile(std::ofstream &fs, int) {
        fs << "(";
        left->compile(fs, 0);
        fs << "/";
        right->compile(fs, 0);
        fs << ")";
}

/******************************************************************************/
/*                             boolean operations                             */
/******************************************************************************/

EqlOP::EqlOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
    : BinaryOperation(left, right) {}

void EqlOP::display() {
        std::cout << "EqlOP(";
        BinaryOperation::display();
}

void EqlOP::compile(std::ofstream &fs, int) {
        left->compile(fs, 0);
        fs << "==";
        right->compile(fs, 0);
}

SupOP::SupOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
    : BinaryOperation(left, right) {}

void SupOP::display() {
        std::cout << "SupOP(";
        BinaryOperation::display();
}

void SupOP::compile(std::ofstream &fs, int) {
        left->compile(fs, 0);
        fs << ">";
        right->compile(fs, 0);
}

InfOP::InfOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
    : BinaryOperation(left, right) {}

void InfOP::display() {
        std::cout << "InfOP(";
        BinaryOperation::display();
}

void InfOP::compile(std::ofstream &fs, int) {
        left->compile(fs, 0);
        fs << "<";
        right->compile(fs, 0);
}

SeqOP::SeqOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
    : BinaryOperation(left, right) {}

void SeqOP::display() {
        std::cout << "SeqOP(";
        BinaryOperation::display();
}

void SeqOP::compile(std::ofstream &fs, int) {
        left->compile(fs, 0);
        fs << ">=";
        right->compile(fs, 0);
}

IeqOP::IeqOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
    : BinaryOperation(left, right) {}

void IeqOP::display() {
        std::cout << "IeqOP(";
        BinaryOperation::display();
}

void IeqOP::compile(std::ofstream &fs, int) {
        left->compile(fs, 0);
        fs << "<=";
        right->compile(fs, 0);
}

OrOP::OrOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
    : BinaryOperation(left, right) {}

void OrOP::display() {
        std::cout << "OrOP(";
        BinaryOperation::display();
}

void OrOP::compile(std::ofstream &fs, int) {
        left->compile(fs, 0);
        fs << " or ";
        right->compile(fs, 0);
}

AndOP::AndOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
    : BinaryOperation(left, right) {}

void AndOP::display() {
        std::cout << "AndOP(";
        BinaryOperation::display();
}

void AndOP::compile(std::ofstream &fs, int) {
        left->compile(fs, 0);
        fs << " and ";
        right->compile(fs, 0);
}

XorOP::XorOP(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
    : BinaryOperation(left, right) {}

void XorOP::display() {
        std::cout << "XorOP(";
        BinaryOperation::display();
}

void XorOP::compile(std::ofstream &fs, int) {
        left->compile(fs, 0);
        fs << " and ";
        right->compile(fs, 0);
}

NotOP::NotOP(std::shared_ptr<Node> param) : param(param) {}

void NotOP::display() {
        std::cout << "NotOP(";
        param->display();
        std::cout << ")";
}

void NotOP::compile(std::ofstream &fs, int) {
        fs << "not(";
        param->compile(fs, 0);
        fs << ") ";
}

/******************************************************************************/
/*                                     IO                                     */
/******************************************************************************/

Print::Print(std::shared_ptr<Node> content) : str(""), content(content) {}

Print::Print(std::string str)
    : str(str), content(std::shared_ptr<Node>(nullptr)) {}

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

Read::Read(std::shared_ptr<TypedNode> variable) : variable(variable) {}

void Read::display() {
        std::cout << "Read(";
        variable->display();
        std::cout << ")" << std::endl;
}

void Read::compile(std::ofstream &fs, int lvl) {
        indent(fs, lvl);
        variable->compile(fs, 0);
        switch (variable->type()) {
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

Return::Return(std::shared_ptr<Node> returnExpr) : returnExpr(returnExpr) {}

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
