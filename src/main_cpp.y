%{
#include <iostream>
#include <string>
#include <cstring>
#include <FlexLexer.h>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "ast/ast.hpp"
#include "symtable/symtable.hpp"
#include "symtable/symbol.hpp"
#include "symtable/contextmanager.hpp"
#include "tools/programbuilder.hpp"
#include "tools/errormanager.hpp"
#include "preprocessor/preprocessor.hpp"
#define YYLOCATION_PRINT   location_print
#define YYDEBUG 1
#define DBG_PARS 0
#if DBG_PARS == 1
#define DEBUG(A) std::cout << A << std::endl
#else
#define DEBUG(A)
#endif
#define PREPROCESSOR_OUTPUT_FILE "__main_pp.prog__"
#define REMOVE_PREPROCESSOR_FILE false
%}
%language "c++"
%defines "parser.hpp"
%output "parser.cpp"

%define api.parser.class {Parser}
%define api.namespace {interpreter}
%define api.value.type variant
%locations
%parse-param {Scanner* scanner} {ProgramBuilder& pb}

%code requires
{
    #include "ast/ast.hpp"
    #include "tools/programbuilder.hpp"
    namespace interpreter {
        class Scanner;
    }
}

%code
{
    #include "tools/checks.hpp"
    #include "lexer.hpp"
    #include <memory>
    #include <functional>
    // #define yylex(x) scanner->lex(x)
    #define yylex(x, y) scanner->lex(x, y) // now we use yylval and yylloc
    Symtable symtable;
    ContextManager contextManager;
    ErrorManager errMgr;

    std::list<std::pair<std::shared_ptr<FunctionCall>, std::pair<std::string, int>>> funcallsToCheck = {};
    std::list<std::pair<std::shared_ptr<Assignment>, std::pair<std::string, int>>> assignmentsToCheck = {};
    std::list<std::pair<std::string, std::function<bool(type_system::type)>>> expressionsToCheck = {};
}

%token <long long>  INT
%token <double>     FLT
%token <char>       CHR
%token NIL
%token INTT FLTT CHRT
%token CND ELS FOR WHL
%token COMMA OSQUAREB CSQUAREB
%token SHW IPT ADD MNS TMS DIV RNG SET
%token EQL SUP INF SEQ IEQ AND LOR XOR NOT
%token <std::string> IDENTIFIER
%token <std::string> STRING
%token ERROR
%token RET
%token BGN END
%token TEXT
%token <std::string> PREPROCESSOR_LOCATION

%nterm <type_system::type> type
%nterm <std::shared_ptr<Value>> value
%nterm <std::shared_ptr<TypedNode>> expression
%nterm <std::shared_ptr<TypedNode>> variable
%nterm <std::shared_ptr<TypedNode>> arithmeticOperation
%nterm <std::shared_ptr<TypedNode>> functionCall
%nterm <std::shared_ptr<Node>> booleanOperation
%nterm <std::shared_ptr<Block>> block
%nterm <std::shared_ptr<Cnd>> cnd
%nterm <std::shared_ptr<Cnd>> cndBase
%nterm <std::shared_ptr<For>> for
%nterm <std::shared_ptr<Whl>> whl

%start start

%%
start: program;

program: %empty | programUnit program ;

programUnit:
    functionDefinition {
        DEBUG("create new function" );
    }
    | PREPROCESSOR_LOCATION {
        // this line is inserted by the preprcessor and allow to know
        // the current file name. To avoid conflicts in lexer's rules we
        // use the string token, however there must be a better way.
        pb.currFileName($1);
    }
    ;

returnTypeSpecifier:
    type[rt] {
        pb.currFunctionReturnType($rt);
    }
    | NIL {
        pb.currFunctionReturnType(type_system::make_type<type_system::Primitive>(type_system::NIL));
    }
    ;

functionDefinition:
    returnTypeSpecifier IDENTIFIER[name] {
        pb.currFunctionName($name);
        std::optional<Symbol> sym = contextManager.lookup($name);
        // error on function redefinition
        if (sym.has_value()) {
            errMgr.addMultipleDefinitionError(pb.currFileName(), @name.begin.line,
                                              $name);
            // TODO: print the previous definition location
            return 1;
        }
        contextManager.enterScope();
    } '('parameterDeclarationList')' {
        type_system::type funType = type_system::make_type<type_system::Function>(
            pb.currFunctionReturnType(),
            pb.getParamsTypes()
        );
        contextManager.newGlobalSymbol(pb.currFunctionName(), funType, FUNCTION);
    } block[ops] {
        // error if there is a return statement
        pb.createFunction(pb.currFunctionName(), $ops, pb.currFunctionReturnType());
        contextManager.leaveScope();
    }
    ;

parameterDeclarationList:
    %empty
    | parameterDeclaration
    | parameterDeclaration COMMA parameterDeclarationList
    ;

parameterDeclaration:
    type[t] IDENTIFIER {
        DEBUG("new param: " << $2);
        contextManager.newSymbol($2, $t, FUN_PARAM);
        pb.pushFunctionParam(Variable($2, $t));
    }
    | type[t] IDENTIFIER OSQUAREB INT[size] CSQUAREB {
        DEBUG("new param: " << $2);
        // TODO: remove the size of the array. The size should be set at
        // -1 (or any default value) in order to specify that we don't
        // want to check the size at compile time when we treat the
        // function
        contextManager.newSymbol($2, $t, LOCAL_ARRAY);
        pb.pushFunctionParam(Array($2, $size,
            std::static_pointer_cast<type_system::StaticArray>($t)));
    }
    ;

parameterList:
    %empty
    | parameter
    | parameter COMMA parameterList
    ;

parameter:
    expression {
        pb.pushFuncallParam($1);
    }
    ;

type:
    INTT {
        $$ = type_system::make_type<type_system::Primitive>(type_system::INT);
    }
    | FLTT {
        $$ = type_system::make_type<type_system::Primitive>(type_system::FLT);
    }
    | CHRT {
        $$ = type_system::make_type<type_system::Primitive>(type_system::CHR);
    }
    ;

block:
    BGN {
        pb.beginBlock();
    } code END {
        DEBUG("new block");
        $$ = pb.endBlock();
    }
    ;

code:
    %empty
    | statement code
    | instruction code
    | RET expression[rs] {
        std::optional<Symbol> sym = contextManager.lookup(pb.currFunctionName());
        type_system::type foundType = $rs->type;
        type_system::type expectedType = sym.value().getType();
        type_system::PrimitiveTypes e_expectedType = expectedType->getEvaluatedType();
        type_system::PrimitiveTypes e_foundType = foundType->getEvaluatedType();
        std::ostringstream oss;

        if (e_expectedType == type_system::NIL) { // no return allowed
            errMgr.addUnexpectedReturnError(pb.currFileName(), @1.begin.line,
                                            pb.currFunctionName());
        } else if (e_expectedType != e_foundType && e_foundType != type_system::NIL) {
            // TODO: create a function to compare types
            std::cout << "ERROR: type comparison done wrong." << std::endl;
            // must check if foundType is not void because of the
            // buildin function (add, ...) which are not in the
            // symtable
            errMgr.addReturnTypeWarning(pb.currFileName(), @1.begin.line,
                                        pb.currFunctionName(), foundType, expectedType);
        }
        // else verify the type and throw a warning
        pb.pushBlock(std::make_shared<Return>($rs));
    }
    ;

instruction:
    shw
    | ipt
    | variableDeclaration
    | assignment
    | functionCall { pb.pushBlock($1); }
    ;

ipt:
    IPT'('variable[c]')' {
        DEBUG("ipt var");
        pb.pushBlock(std::make_shared<Ipt>($c));
    }
    ;

shw:
    SHW'('expression[ic]')' {
        DEBUG("shw var");
        // spcial case for strings
        if (type_system::isArrayOfChr($ic->type)) {
            auto stringValue = std::dynamic_pointer_cast<Value>($ic);
            std::string str = stringValue->value._str;
            pb.pushBlock(std::make_shared<Shw>(str));
        } else {
            pb.pushBlock(std::make_shared<Shw>($ic));
        }
    }
    ;

expression:
    arithmeticOperation { $$ = $1; }
    | functionCall { $$ = $1; }
    | value { $$ = $1; }
    | variable { $$ = $1; }
    ;

variable:
    IDENTIFIER {
        DEBUG("new param variable");
        type_system::type type;
        std::shared_ptr<Variable> v;

        // TODO: this is really bad, the function isDefined will be
        // changed !
        if (isDefined(pb.currFileName(), @1.begin.line, $1, type)) {
            if (isArray(type)) {
                Symbol sym = contextManager.lookup($1).value();
                v = std::make_shared<Array>($1, getArraySize(sym.getType()), type);
            } else {
                v = std::make_shared<Variable>($1, type);
            }
        } else {
            v = std::make_shared<Variable>($1, type_system::make_type<type_system::Primitive>(type_system::NIL));
        }
        $$ = v;
    }
    | IDENTIFIER OSQUAREB expression[index] CSQUAREB {
        DEBUG("using an array");
        type_system::type type;
        std::shared_ptr<ArrayAccess> v;
        // TODO: refactor isDefined
        if (isDefined(pb.currFileName(), @1.begin.line, $1, type)) {
            std::optional<Symbol> sym = contextManager.lookup($1);
            // error if the symbol is not an array
            if (sym.value().getKind() != LOCAL_ARRAY) {
                errMgr.addBadArrayUsageError(pb.currFileName(), @1.begin.line, $1);
            }
            v = std::make_shared<ArrayAccess>($1, type, $index);
        } else {
            // TODO: verify the type of the index
            v = std::make_shared<ArrayAccess>($1,
                type_system::make_type<type_system::Primitive>(type_system::NIL), $index);
        }
        $$ = v;
    }
    ;

arithmeticOperation:
    ADD'(' expression[left] COMMA expression[right] ')' {
        DEBUG("addOP");
        // todo: create a template function make_arithmetic_operator (if one of
        // the expressions is an undefined function, add the expression to the
        // expression to check)
        if (!isNumber($left->type) || !isNumber($right->type)) {
            errMgr.addOperatorError(pb.currFileName(), @1.begin.line, "add");
        }
        $$ = std::make_shared<AddOP>($left, $right);
    }
    | MNS'(' expression[left] COMMA expression[right] ')' {
        DEBUG("mnsOP");
        if (!isNumber($left->type) || !isNumber($right->type)) {
            errMgr.addOperatorError(pb.currFileName(), @1.begin.line, "mns");
        }
        $$ = std::make_shared<MnsOP>($left, $right);
    }
    | TMS'(' expression[left] COMMA expression[right] ')' {
        DEBUG("tmsOP");
        if (!isNumber($left->type) || !isNumber($right->type)) {
            errMgr.addOperatorError(pb.currFileName(), @1.begin.line, "tms");
        }
        $$ = std::make_shared<TmsOP>($left, $right);
    }
    | DIV'(' expression[left] COMMA expression[right] ')' {
        DEBUG("divOP");
        $$ = std::make_shared<DivOP>($left, $right);
        if (!isNumber($left->type) || !isNumber($right->type)) {
            errMgr.addOperatorError(pb.currFileName(), @1.begin.line, "div");
        }
    }
    ;

booleanOperation:
    EQL'(' expression[left] COMMA expression[right] ')' {
        DEBUG("EqlOP");
        $$ = std::make_shared<EqlOP>($left, $right);
    }
    | SUP'(' expression[left] COMMA expression[right] ')' {
        DEBUG("SupOP");
        $$ = std::make_shared<SupOP>($left, $right);
    }
    | INF'(' expression[left] COMMA expression[right] ')' {
        DEBUG("InfOP");
        $$ = std::make_shared<InfOP>($left, $right);
    }
    | SEQ'(' expression[left] COMMA expression[right] ')' {
        DEBUG("SeqOP");
        $$ = std::make_shared<SeqOP>($left, $right);
    }
    | IEQ'(' expression[left] COMMA expression[right] ')' {
        DEBUG("IeqOP");
        $$ = std::make_shared<IeqOP>($left, $right);
    }
    | AND'('booleanOperation[left] COMMA booleanOperation[right]')' {
        DEBUG("AndOP");
        $$ = std::make_shared<AndOP>($left, $right);
    }
    | LOR'('booleanOperation[left] COMMA booleanOperation[right]')' {
        DEBUG("LorOP");
        $$ = std::make_shared<OrOP>($left, $right);
    }
    | XOR'('booleanOperation[left] COMMA booleanOperation[right]')' {
        DEBUG("XorOP");
        $$ = std::make_shared<XorOP>($left, $right);
    }
    | NOT'('booleanOperation[op]')' {
        DEBUG("NotOP");
        $$ = std::make_shared<NotOP>($op);
    }
    ;

functionCall:
    IDENTIFIER'(' {
        pb.newFuncall($1);
    }
    parameterList')' {
        // TODO: we need a none type as if the function is not defined, we
        // cannot get the return type (should be verified afterward)
        std::shared_ptr<FunctionCall> funcall;
        if (std::optional<Symbol> symbol = contextManager.lookup($1)) {
            funcall =
            pb.createFuncall(type_system::make_type<type_system::Primitive>(symbol.value().getType()->getEvaluatedType()));
        } else {
            funcall = pb.createFuncall();
        }
        std::pair<std::string, int> position = std::make_pair(pb.currFileName(), @1.begin.line);
        funcallsToCheck.push_back(std::make_pair(funcall, position));
        // the type check is done at the end !
        DEBUG("new funcall: " << $1);
        // check the type
        $$ = funcall;
    }
    ;

variableDeclaration:
    type[t] IDENTIFIER[name] {
        DEBUG("new declaration: " << $name);
        // redefinitions are not allowed:
        if (std::optional<Symbol> symbol = contextManager.lookup($name)) {
            errMgr.addMultipleDefinitionError(pb.currFileName(), @name.begin.line, $name);
        }
        contextManager.newSymbol($2, $t, LOCAL_VAR);
        pb.pushBlock(std::make_shared<Declaration>(Variable($2, $t)));
    }
    | type[t] IDENTIFIER[name] OSQUAREB INT[size] CSQUAREB {
        DEBUG("new array declaration: " << $2);
        // redefinitions are not allowed:
        if (std::optional<Symbol> symbol = contextManager.lookup($name)) {
            errMgr.addMultipleDefinitionError(pb.currFileName(), @name.begin.line, $name);
        }
        contextManager.newSymbol($name, $t, $size, LOCAL_ARRAY);
        pb.pushBlock(std::make_shared<ArrayDeclaration>($name, $size, $t));
    }
    ;

assignment:
    SET'('variable[c] COMMA expression[ic]')' {
        DEBUG("new assignment");
        auto icType = $ic->type;
        auto v = std::static_pointer_cast<Variable>($c);
        auto newAssignment = std::make_shared<Assignment>(v, $ic);

        if (std::static_pointer_cast<FunctionCall>($ic)) { // if funcall
            // this is a funcall so we have to wait the end of the parsing to check
            auto position = std::make_pair(pb.currFileName(), @c.begin.line);
            assignmentsToCheck.push_back(std::pair(newAssignment, position));
        } else {
            checkType(pb.currFileName(), @c.begin.line, v->id, $c->type, icType->getEvaluatedType());
        }
        pb.pushBlock(newAssignment);
        // TODO: check the type for strings -> array of char
    }
    ;

value:
    INT {
        DEBUG("new int: " << $1);
        type_system::LiteralValue v = { ._int = $1 };
        $$ = std::make_shared<Value>(v, type_system::make_type<type_system::Primitive>(type_system::INT));
    }
    | FLT {
        DEBUG("new double: " << $1);
        type_system::LiteralValue v = { ._flt = $1 };
        $$ = std::make_shared<Value>(v, type_system::make_type<type_system::Primitive>(type_system::FLT));
    }
    | CHR {
        DEBUG("new char: " << $1);
        type_system::LiteralValue v = { ._chr = $1 };
        $$ = std::make_shared<Value>(v, type_system::make_type<type_system::Primitive>(type_system::CHR));
    }
    | STRING {
        DEBUG("new char: " << $1);
        type_system::LiteralValue v = {0};
        if ($1.size() > MAX_LITERAL_STRING_LENGTH) {
            errMgr.addLiteralStringOverflowError(pb.currFileName(), @1.begin.line);
            return 1;
        }
        memcpy(v._str, $1.c_str(), $1.size());
        $$ = std::make_shared<Value>(v, type_system::make_type<type_system::StaticArray>(type_system::CHR, $1.size()));
    }
    ;

statement:
    cnd {
        DEBUG("new if");
        pb.pushBlock($1);
    }
    | for {
        DEBUG("new for");
        pb.pushBlock($1);
    }
    | whl {
        DEBUG("new whl");
        pb.pushBlock($1);
    }
    ;

cnd:
    cndBase {
        $$ = $1;
    }
    | cndBase[cndb] ELS {
        DEBUG("els");
        contextManager.enterScope();
    } block[ops] {
        std::shared_ptr<Cnd> ifstmt = $cndb;
        // adding else block
        ifstmt->elseBlock = $ops;
        $$ = ifstmt;
        contextManager.leaveScope();
    }
    ;

cndBase:
    CND booleanOperation[cond] {
        contextManager.enterScope();
    } block[ops] {
        DEBUG("if");
        $$ = pb.createCnd($cond, $ops);
        contextManager.leaveScope();
    }
    ;

for:
    FOR IDENTIFIER[v] RNG'('expression[b] COMMA expression[e] COMMA expression[s]')' {
        contextManager.enterScope();
    } block[ops] {
        DEBUG("in for");
        Variable v($v, type_system::make_type<type_system::Primitive>(type_system::NIL));
        type_system::type type;
        if (isDefined(pb.currFileName(), @v.begin.line, $v, type)) {
            v = Variable($v, type);
            checkType(pb.currFileName(), @b.begin.line, "RANGE_BEGIN", type, $b->type->getEvaluatedType());
            checkType(pb.currFileName(), @e.begin.line, "RANGE_END",  type, $e->type->getEvaluatedType());
            checkType(pb.currFileName(), @s.begin.line, "RANGE_STEP", type, $s->type->getEvaluatedType());
        }
        $$ = pb.createFor(v, $b, $e, $s, $ops);
        contextManager.leaveScope();
    }
    ;

whl:
    WHL '('booleanOperation[cond]')' {
        contextManager.enterScope();
    } block[ops] {
        DEBUG("in whl");
        $$ = pb.createWhl($cond, $ops);
        contextManager.leaveScope();
    }
    ;
%%

void interpreter::Parser::error(const location_type& loc, const std::string& msg) {
    std::ostringstream oss;
    oss << pb.currFileName() << ":" << loc.begin.line << ": " << msg << "." << std::endl;
    errMgr.addError(oss.str());
}

/* Run interactive parser. It was used during the beginning of the project. */
void cli() {
    ProgramBuilder pb;
    interpreter::Scanner scanner{ std::cin, std::cerr };
    interpreter::Parser parser{ &scanner, pb };
    contextManager.enterScope();
    parser.parse();
    errMgr.report();
    if (!errMgr.getErrors()) {
        pb.display();
    }
}

/* add execution rights to the result file */
void makeExecutable(std::string file) {
    std::filesystem::permissions(file,
            std::filesystem::perms::owner_exec
            | std::filesystem::perms::group_exec
            | std::filesystem::perms::others_exec,
            std::filesystem::perm_options::add);
}

/* Verify the types of all assignments that involve funcalls.
 * It's done because we want to be able to use functions that are declared after
 * the function in which we make the call. This force to parse all the functions
 * to have a complete table of symbol before checking the types.
 */
void checkAssignments() {
    for (auto ap : assignmentsToCheck) {
        checkType(ap.second.first, ap.second.second,
                  ap.first->variable->id,
                  ap.first->variable->type,
                  ap.first->value->type->getEvaluatedType());
    }
}

// todo: we shouldn't need this
type_system::types getTypes(std::list<std::shared_ptr<TypedNode>> const &nodes) {
    type_system::types parametersTypes = {};

    std::transform(nodes.cbegin(), nodes.cend(),
                   std::back_insert_iterator<type_system::types>(parametersTypes),
                   [](auto elt) { return elt->type; });
    return parametersTypes;
}

/* Verify the types of all funcalls. To check the type, we have to verify the
 * types of all the parameters. The return type is not important here.
 */
void checkFuncalls() {
    // std::list<std::pair<std::shared_ptr<FunctionCall>,
    //                     std::pair<std::string, int>>> funcallsToCheck;
    for (auto fp : funcallsToCheck) {
        std::optional<Symbol> sym = contextManager.lookup(fp.first->functionName);

        if (sym.has_value()) {
            // get the found return type (types of the parameters)
            type_system::types foundArgumentsTypes = getTypes(fp.first->parameters);
            type_system::types expectedArgumentsTypes =
                std::static_pointer_cast<type_system::Function>(
                                            sym.value().getType())->argumentsTypes;

            if (!checkParametersTypes(expectedArgumentsTypes, foundArgumentsTypes)) {
                errMgr.addFuncallTypeError(fp.second.first,
                                           fp.second.second,
                                           fp.first->functionName,
                                           expectedArgumentsTypes, foundArgumentsTypes);
            }
        } else {
            // todo
            // errMgr.addUndefinedSymbolError(fp.first->functionName(), fp.second.first, fp.second.second);
        }
    }
}

void compile(std::string fileName, std::string outputName) {
    int parserOutput;
    int preprocessorErrorStatus = 0;

    ProgramBuilder pb;
    Preprocessor pp(PREPROCESSOR_OUTPUT_FILE);

    pb.currFileName(fileName);
    contextManager.enterScope(); // update the scope

    try {
        pp.process(fileName); // launch the preprocessor
    } catch (std::logic_error& e) {
        errMgr.addError(e.what());
        preprocessorErrorStatus = 1;
    }

    // open and parse the file
    std::ifstream is("__main_pp.prog__", std::ios::in); // parse the preprocessed file
    interpreter::Scanner scanner{ is , std::cerr };
    interpreter::Parser parser{ &scanner, pb };
    parserOutput = parser.parse();
    checkFuncalls();
    checkAssignments();

    // look for main
    std::optional<Symbol> sym = contextManager.lookup("main");
    if (0 == parserOutput && 0 == preprocessorErrorStatus && !sym.has_value()) {
        errMgr.addNoEntryPointError();
    }
    // report errors and warnings
    errMgr.report();

    // if no errors, transpile the file
    if (!errMgr.getErrors()) {
        std::ofstream fs(outputName);
        pb.getProgram()->compile(fs);
        makeExecutable(outputName);
    }

    // remove the preprocessor output file
    if (REMOVE_PREPROCESSOR_FILE) {
        std::filesystem::remove(PREPROCESSOR_OUTPUT_FILE);
    }
}

int main(int argc, char **argv) {
    if (argc == 2) {
        compile(argv[1], "a.out"); // TODO: add an option to choose the name of the created script
    } else { // launch the interpreter for debugging
        cli();
    }
}
