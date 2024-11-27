#include "ast.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>

void indent(std::ofstream &fs, int lvl) {
    for (int i = 0; i < lvl; ++i) {
        fs << '\t';
    }
}

/* -------------------------------------------------------------------------- */

void Value::display() {
    // todo: we can't use getElementType because of the strings
    switch (type_system::getPrimitiveType(type_system::getElementType(type))
                .value()) {
    case type_system::INT:
        std::cout << value._int;
        break;
    case type_system::FLT:
        std::cout << value._flt;
        break;
    case type_system::CHR:
        std::cout << "'" << value._chr << "'";
        break;
    default:
        break;
    }
}

void Value::compile(std::ofstream &fs, int) {
    // todo: we can't use getElementType because of the strings
    switch (type->kind) {
    case type_system::TypeKinds::Primitive:
        switch (type_system::getPrimitiveType(type_system::getElementType(type))
                    .value()) {
        case type_system::INT:
            fs << value._int;
            break;
        case type_system::FLT:
            fs << value._flt;
            break;
        case type_system::CHR:
            fs << "'" << value._chr << "'";
            break;
        default:
            // TODO: proper error
            throw std::runtime_error("error: unknown primitive type.");
            break;
        }
        break;
    case type_system::TypeKinds::StaticArray:
        if (type_system::getPrimitiveType(type_system::getElementType(type))
                .value() == type_system::CHR) {
            // WARN: the '"' are in the string (this may change).
            // TODO: this doesn't work, the value is technically correct but
            // it doesn't take in count the size of the targeted array.
            std::string str = value._str;
            fs << "[c for c in " << str << "]+[0]";
        }
        break;
    default:
        // TODO: proper error
        throw std::runtime_error("error: unsupported value type.");
        break;
    }
}

/* -------------------------------------------------------------------------- */

void Variable::display() { std::cout << id; }

void Variable::compile(std::ofstream &fs, int) { fs << id; }

/* -------------------------------------------------------------------------- */

void ArrayDeclaration::display() { std::cout << id << "[" << size << "]"; }

void ArrayDeclaration::compile(std::ofstream &fs, int lvl) {
    indent(fs, lvl);
    fs << id << "=[0 for _ in range(" << size << ")]";
}

void ArrayAccess::display() {
    std::cout << Variable::id << "[";
    index->display();
    std::cout << "]";
}

void ArrayAccess::compile(std::ofstream &fs, int) {
    fs << Variable::id << "[";
    index->compile(fs, 0);
    fs << "]";
}

/* -------------------------------------------------------------------------- */

void Function::display() {
    std::cout << "Function(" << id << ", [";
    for (Variable p : parameters) {
        p.display();
        std::cout << ", ";
    }
    std::cout << "], ";
    block->display();
    std::cout << ")" << std::endl;
}

void Function::compile(std::ofstream &fs, int) {
    fs << "def " << id << "(";
    if (parameters.size() > 0) {
        std::list<Variable> tmp = parameters;
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

void Block::display() {
    std::cout << "Block(" << std::endl;
    for (std::shared_ptr<Node> o : instructions) {
        o->display();
    }
    std::cout << ")" << std::endl;
}

void Block::compile(std::ofstream &fs, int lvl) {
    for (std::shared_ptr<Node> op : instructions) {
        op->compile(fs, lvl + 1);
        fs << std::endl;
    }
}

/* -------------------------------------------------------------------------- */

void Assignment::display() {
    std::cout << "Assignment(";
    variable->display();
    std::cout << ",";
    value->display();
    std::cout << ")" << std::endl;
}

void Assignment::compile(std::ofstream &fs, int lvl) {
    indent(fs, lvl);
    if (variable->type->kind == type_system::TypeKinds::StaticArray) {
        std::shared_ptr<Array> array =
            std::dynamic_pointer_cast<Array>(variable);
        std::shared_ptr<Value> val = std::dynamic_pointer_cast<Value>(value);
        // WARN: the value contains the '"'
        std::string str = val->value._str;
        // TODO: this should be done at runtime !
        unsigned int size = std::min(array->size, (int)str.size() - 2 + 1);

        // reset the array before assignment of the string
        fs << array->id << "=[0 for _ in range(" << array->size << ")]"
           << std::endl;
        indent(fs, lvl);
        fs << "for _ZZ_TRANSPILER_STRINGSET_INDEX in range(" << size - 1
           << "):" << std::endl;
        indent(fs, lvl + 1);
        fs << variable->id << "[_ZZ_TRANSPILER_STRINGSET_INDEX]=";
        fs << str << "[_ZZ_TRANSPILER_STRINGSET_INDEX]";
    } else {
        variable->compile(fs, lvl);
        fs << "=";
        switch (type_system::getPrimitiveType(
                    type_system::getElementType(variable->type))
                    .value()) {
        case type_system::INT:
            fs << "int(";
            break;
        case type_system::CHR:
            fs << "chr(";
            break;
        case type_system::FLT:
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

void Declaration::display() {
    std::cout << "Declaration(";
    variable.display();
    std::cout << ")" << std::endl;
}

void Declaration::compile(std::ofstream &fs, int lvl) {
    indent(fs, lvl);
    fs << "# " << variable.type << " " << variable.id;
}

/* -------------------------------------------------------------------------- */

void FunctionCall::display() {
    std::cout << "Funcall(" << functionName << ", [";
    for (std::shared_ptr<Node> p : parameters) {
        p->display();
        std::cout << ", ";
    }
    std::cout << "])" << std::endl;
}

void FunctionCall::compile(std::ofstream &fs, int lvl) {
    // TODO: there is more work to do when we pas a string to the function
    indent(fs, lvl);
    fs << functionName << "(";
    for (std::shared_ptr<Node> p : parameters) {
        p->compile(fs, 0);
        if (p != parameters.back())
            fs << ',';
    }
    fs << ")";
}

/******************************************************************************/
/*                                 statements                                 */
/******************************************************************************/

/* -------------------------------------------------------------------------- */

void Cnd::display() {
    std::cout << "If(";
    condition->display();
    std::cout << ", ";
    block->display();
    if (elseBlock != nullptr) { // print else block if needed
        std::cout << ", Else(";
        elseBlock->display();
        std::cout << ")" << std::endl;
    }
    std::cout << ")" << std::endl;
}

void Cnd::compile(std::ofstream &fs, int lvl) {
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

void For::display() {
    std::cout << "For(";
    variable.display();
    std::cout << ", range(";
    begin->display();
    std::cout << ",";
    end->display();
    std::cout << ",";
    step->display();
    std::cout << "), ";
    block->display();
    std::cout << ")" << std::endl;
}

void For::compile(std::ofstream &fs, int lvl) {
    // TODO: vérifier les type et cast si besoin
    indent(fs, lvl);
    fs << "for ";
    variable.compile(fs, 0);
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

void Whl::display() {
    std::cout << "While(";
    condition->display();
    std::cout << ", ";
    block->display();
    std::cout << ")" << std::endl;
}

void Whl::compile(std::ofstream &fs, int lvl) {
    indent(fs, lvl);
    fs << "while ";
    condition->compile(fs, 0);
    fs << ":" << std::endl;
    block->compile(fs, lvl);
}

/******************************************************************************/
/*                           arithemtic operations                            */
/******************************************************************************/

void BinaryOperation::display() {
    left->display();
    std::cout << ", ";
    right->display();
    std::cout << ")";
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

void EqlOP::display() {
    std::cout << "EqlOP(";
    BinaryOperation::display();
}

void EqlOP::compile(std::ofstream &fs, int) {
    left->compile(fs, 0);
    fs << "==";
    right->compile(fs, 0);
}

void SupOP::display() {
    std::cout << "SupOP(";
    BinaryOperation::display();
}

void SupOP::compile(std::ofstream &fs, int) {
    left->compile(fs, 0);
    fs << ">";
    right->compile(fs, 0);
}

void InfOP::display() {
    std::cout << "InfOP(";
    BinaryOperation::display();
}

void InfOP::compile(std::ofstream &fs, int) {
    left->compile(fs, 0);
    fs << "<";
    right->compile(fs, 0);
}

void SeqOP::display() {
    std::cout << "SeqOP(";
    BinaryOperation::display();
}

void SeqOP::compile(std::ofstream &fs, int) {
    left->compile(fs, 0);
    fs << ">=";
    right->compile(fs, 0);
}

void IeqOP::display() {
    std::cout << "IeqOP(";
    BinaryOperation::display();
}

void IeqOP::compile(std::ofstream &fs, int) {
    left->compile(fs, 0);
    fs << "<=";
    right->compile(fs, 0);
}

void OrOP::display() {
    std::cout << "OrOP(";
    BinaryOperation::display();
}

void OrOP::compile(std::ofstream &fs, int) {
    left->compile(fs, 0);
    fs << " or ";
    right->compile(fs, 0);
}

void AndOP::display() {
    std::cout << "AndOP(";
    BinaryOperation::display();
}

void AndOP::compile(std::ofstream &fs, int) {
    left->compile(fs, 0);
    fs << " and ";
    right->compile(fs, 0);
}

void XorOP::display() {
    std::cout << "XorOP(";
    BinaryOperation::display();
}

void XorOP::compile(std::ofstream &fs, int) {
    left->compile(fs, 0);
    fs << " and ";
    right->compile(fs, 0);
}

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

void Shw::display() {
    std::cout << "Print(";
    if (content != nullptr) {
        content->display();
    } else {
        std::cout << str;
    }
    std::cout << ");" << std::endl;
}

void Shw::compile(std::ofstream &fs, int lvl) {
    indent(fs, lvl);
    fs << "print(";
    if (content == nullptr) {
        fs << str;
    } else {
        content->compile(fs, 0);
    }
    fs << ",end=\"\")";
}

void Ipt::display() {
    std::cout << "Read(";
    variable->display();
    std::cout << ")" << std::endl;
}

void Ipt::compile(std::ofstream &fs, int lvl) {
    indent(fs, lvl);
    variable->compile(fs, 0);
    // todo: make shura that the variable is a primitive type
    switch (type_system::getPrimitiveType(
                type_system::getElementType(variable->type))
                .value()) {
    case type_system::INT:
        fs << " = int(input())";
        break;
    case type_system::FLT:
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
