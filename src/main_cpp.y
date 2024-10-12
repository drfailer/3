%{
#include <iostream>
#include <string>
#include <cstring>
#include <FlexLexer.h>
#include <fstream>
#include <filesystem>
#include "ast/AST.hpp"
#include "ast/ProgramBuilder.hpp"
#include "symtable/Symtable.hpp"
#include "symtable/Symbol.hpp"
#include "symtable/ContextManager.hpp"
#include "errorManager/ErrorManager.hpp"
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
    #include "ast/AST.hpp"
    #include "ast/ProgramBuilder.hpp"
    namespace interpreter {
        class Scanner;
    }
}

%code
{
    #include "checks.hpp"
    #include "lexer.hpp"
    #include <memory>
    // #define yylex(x) scanner->lex(x)
    #define yylex(x, y) scanner->lex(x, y) // now we use yylval and yylloc
    Symtable symtable;
    ContextManager contextManager;
    ErrorManager errMgr;
    std::string currentFunctionName = "";
    Type currentFunctionReturnType = VOID;
    std::string currentFile = "";

    std::list<std::pair<std::shared_ptr<Funcall>, std::pair<std::string, int>>> funcallsToCheck;
    std::list<std::pair<std::shared_ptr<Assignment>, std::pair<std::string, int>>> assignmentsToCheck;
}

%token <long long>  INT
%token <double>     FLOAT
%token <char>       CHAR
/* %nonassoc           ASSIGN */
%token INTT FLTT CHRT
%token IF ELSE FOR WHILE FN IN
%token SEMI COMMA ARROW OSQUAREB CSQUAREB
%token PRINT READ ADD MNS TMS DIV RANGE SET
%token EQL SUP INF SEQ IEQ AND OR XOR NOT
%token <std::string> IDENTIFIER
%token <std::string> STRING
%token ERROR
%token RETURN
%token HASH EOL TEXT
%token <std::string> PREPROCESSOR_LOCATION

%nterm <Type> type
%nterm <Value> value
%nterm <std::shared_ptr<TypedElement>> expression
%nterm <std::shared_ptr<TypedElement>> variable
%nterm <std::shared_ptr<TypedElement>> arithmeticOperation
%nterm <std::shared_ptr<ASTNode>> booleanOperation
%nterm <std::shared_ptr<TypedElement>> functionCall
%nterm <std::shared_ptr<Block>> block
%nterm <std::shared_ptr<If>> if
%nterm <std::shared_ptr<If>> simpleIf
%nterm <std::shared_ptr<For>> for
%nterm <std::shared_ptr<While>> while

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
        currentFile = $1;
        // suppression des '"':
        currentFile.erase(0, 1);
        currentFile.pop_back();
    }
    ;

returnTypeSpecifier:
    ARROW type[rt] {
        currentFunctionReturnType = $rt;
    }
    | %empty { currentFunctionReturnType = VOID; }
    ;

functionDefinition:
    FN IDENTIFIER[name] {
        currentFunctionName = $name;
        std::optional<Symbol> sym = contextManager.lookup($name);
        // error on function redefinition
        if (sym.has_value()) {
            errMgr.addMultipleDefinitionError(currentFile, @name.begin.line,
                                              $name);
            // TODO: print the previous definition location
            return 1;
        }
        contextManager.enterScope();
    } '('parameterDeclarationList')' returnTypeSpecifier {
        std::list<Type> funType = pb.getParamsTypes();
        funType.push_back(currentFunctionReturnType);
        contextManager.newGlobalSymbol(currentFunctionName, funType, FUNCTION);
    } block[ops] {
        // error if there is a return statement
        pb.createFunction(currentFunctionName, $ops, currentFunctionReturnType);
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
        contextManager.newSymbol($2, std::list<Type>($t), FUN_PARAM);
        pb.pushFunctionParam(Variable($2, $t));
    }
    | type[t] IDENTIFIER OSQUAREB INT[size] CSQUAREB {
        DEBUG("new param: " << $2);
        // TODO: remove the size of the array. The size should be set at
        // -1 (or any default value) in order to specify that we don't
        // want to check the size at compile time when we treat the
        // function
        contextManager.newSymbol($2, std::list<Type>($t), LOCAL_ARRAY);
        pb.pushFunctionParam(Array($2, $size, getArrayType($t)));
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
        $$ = INT;
    }
    | FLTT {
        $$ = FLT;
    }
    | CHRT {
        $$ = CHR;
    }
    ;

block:
    '{' {
        pb.beginBlock();
    } code '}' {
        DEBUG("new block");
        $$ = pb.endBlock();
    }
    ;

code:
    %empty
    | statement code
    | instruction SEMI code
    | RETURN expression[rs] SEMI {
        std::optional<Symbol> sym = contextManager.lookup(currentFunctionName);
        Type foundType = $rs->getType();
        Type expectedType = sym.value().getType().back();
        std::ostringstream oss;

        if (expectedType == VOID) { // no return allowed
            errMgr.addUnexpectedReturnError(currentFile, @1.begin.line,
                                            currentFunctionName);
        } else if (expectedType != foundType && foundType != VOID) {
            // must check if foundType is not void because of the
            // buildin function (add, ...) which are not in the
            // symtable
            errMgr.addReturnTypeWarning(currentFile, @1.begin.line,
                                        currentFunctionName, foundType, expectedType);
        }
        // else verify the type and throw a warning
        pb.pushBlock(std::make_shared<Return>($rs));
    }
    ;

instruction:
    print
    | read
    | variableDeclaration
    | assignment
    | functionCall { pb.pushBlock($1); }
    ;

read:
    READ'('variable[c]')' {
        DEBUG("read");
        pb.pushBlock(std::make_shared<Read>($c));
    }
    ;

print:
    PRINT'('STRING[s]')' {
        DEBUG("print str");
        pb.pushBlock(std::make_shared<Print>($s));
    }
    | PRINT'('expression[ic]')' {
        DEBUG("print var");
        pb.pushBlock(std::make_shared<Print>($ic));
    }
    ;

expression:
    arithmeticOperation { $$ = $1; }
    | functionCall { $$ = $1; }
    | value { $$ = std::make_shared<Value>($1); }
    | variable { $$ = $1; }
    ;

variable:
    IDENTIFIER {
        DEBUG("new param variable");
        std::list<Type> type;
        std::shared_ptr<Variable> v;

        // TODO: this is really bad, the function isDefined will be
        // changed !
        if (isDefined(currentFile, @1.begin.line, $1, type)) {
            if (isArray(type.back())) {
                Symbol sym = contextManager.lookup($1).value();
                v = std::make_shared<Array>($1, sym.getSize(), type.back());
            } else {
                v = std::make_shared<Variable>($1, type.back());
            }
        } else {
                v = std::make_shared<Variable>($1, VOID);
        }
        $$ = v;
    }
    | IDENTIFIER OSQUAREB expression[index] CSQUAREB {
        DEBUG("using an array");
        std::list<Type> type;
        std::shared_ptr<ArrayAccess> v;
        // TODO: refactor isDefined
        if (isDefined(currentFile, @1.begin.line, $1, type)) {
            std::optional<Symbol> sym = contextManager.lookup($1);
            // error if the symbol is not an array
            if (sym.value().getKind() != LOCAL_ARRAY) {
                errMgr.addBadArrayUsageError(currentFile, @1.begin.line, $1);
            }
            v = std::make_shared<ArrayAccess>($1, getValueType(type.back()), $index);
        } else {
            // TODO: verify the type of the index
            v = std::make_shared<ArrayAccess>($1, VOID, $index);
        }
        $$ = v;
    }
    ;

arithmeticOperation:
    ADD'(' expression[left] COMMA expression[right] ')' {
        DEBUG("addOP");
        if (!isNumber($left->getType()) || !isNumber($right->getType())) {
            errMgr.addOperatorError(currentFile, @1.begin.line, "add");
        }
        $$ = std::make_shared<AddOP>($left, $right);
    }
    | MNS'(' expression[left] COMMA expression[right] ')' {
        DEBUG("mnsOP");
        if (!isNumber($left->getType()) || !isNumber($right->getType())) {
            errMgr.addOperatorError(currentFile, @1.begin.line, "mns");
        }
        $$ = std::make_shared<MnsOP>($left, $right);
    }
    | TMS'(' expression[left] COMMA expression[right] ')' {
        DEBUG("tmsOP");
        if (!isNumber($left->getType()) || !isNumber($right->getType())) {
            errMgr.addOperatorError(currentFile, @1.begin.line, "tms");
        }
        $$ = std::make_shared<TmsOP>($left, $right);
    }
    | DIV'(' expression[left] COMMA expression[right] ')' {
        DEBUG("divOP");
        $$ = std::make_shared<DivOP>($left, $right);
        if (!isNumber($left->getType()) || !isNumber($right->getType())) {
            errMgr.addOperatorError(currentFile, @1.begin.line, "div");
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
    | OR'('booleanOperation[left] COMMA booleanOperation[right]')' {
        DEBUG("OrOP");
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
        std::shared_ptr<Funcall> funcall = pb.createFuncall();
        // TODO: save the funcall and params in a vector (create a struct)
        funcall->setType(VOID); // type to VOID by default, will change on the type check
        std::pair<std::string, int> position = std::make_pair(currentFile, @1.begin.line);
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
            errMgr.addMultipleDefinitionError(currentFile, @name.begin.line, $name);
        }
        std::list<Type> t;
        t.push_back($t);
        contextManager.newSymbol($2, t, LOCAL_VAR);
        pb.pushBlock(std::make_shared<Declaration>(Variable($2, $t)));
    }
    | type[t] IDENTIFIER[name] OSQUAREB INT[size] CSQUAREB {
        DEBUG("new array declaration: " << $2);
        // redefinitions are not allowed:
        if (std::optional<Symbol> symbol = contextManager.lookup($name)) {
            errMgr.addMultipleDefinitionError(currentFile, @name.begin.line, $name);
        }
        std::list<Type> t;
        t.push_back(getArrayType($t));
        contextManager.newSymbol($name, t, $size, LOCAL_ARRAY);
        pb.pushBlock(std::make_shared<ArrayDeclaration>($name, $size, getArrayType($t)));
    }
    ;

assignment:
    SET'('variable[c] COMMA expression[ic]')' {
        DEBUG("new assignment");
        Type icType = $ic->getType();
        std::shared_ptr<Variable> v = std::static_pointer_cast<Variable>($c);
        std::shared_ptr<Assignment> newAssignment = std::make_shared<Assignment>(v, $ic);

        if (std::static_pointer_cast<Funcall>($ic)) { // if funcall
            // this is a funcall so we have to wait the end of the parsing to check
            std::pair<std::string, int> position = std::make_pair(currentFile, @c.begin.line);
            assignmentsToCheck.push_back(std::pair(newAssignment, position));
        } else {
            checkType(currentFile, @c.begin.line, v->getId(), $c->getType(), icType);
        }
        pb.pushBlock(newAssignment);
        // TODO: check the type for strings -> array of char
    }
    ;

value:
    INT {
        DEBUG("new int: " << $1);
        type_t v = { .i = $1 };
        $$ = Value(v, INT);
    }
    | FLOAT {
        DEBUG("new double: " << $1);
        type_t v = { .f = $1 };
        $$ = Value(v, FLT);
    }
    | CHAR {
        DEBUG("new char: " << $1);
        type_t v = { .c = $1 };
        $$ = Value(v, CHR);
    }
    | STRING {
        DEBUG("new char: " << $1);
        type_t v = {0};
        memcpy(v.s, $1.c_str(), $1.size());
        $$ = Value(v, ARR_CHR);
    }
    ;

statement:
    if {
        DEBUG("new if");
        pb.pushBlock($1);
    }
    | for {
        DEBUG("new for");
        pb.pushBlock($1);
    }
    | while {
        DEBUG("new for");
        pb.pushBlock($1);
    }
    ;

if:
    simpleIf {
        $$ = $1;
    }
    | simpleIf[sif] ELSE {
        contextManager.enterScope();
    } block[ops] {
        DEBUG("else");
        std::shared_ptr<If> ifstmt = $sif;
        // adding else block
        ifstmt->createElse($ops);
        $$ = ifstmt;
        contextManager.leaveScope();
    }
    ;

simpleIf:
    IF '('booleanOperation[cond]')' {
        contextManager.enterScope();
    } block[ops] {
        DEBUG("if");
        $$ = pb.createIf($cond, $ops);
        contextManager.leaveScope();
    }
    ;

for:
    FOR IDENTIFIER[v] IN RANGE'('expression[b] COMMA expression[e] COMMA expression[s]')' {
        contextManager.enterScope();
    } block[ops] {
        DEBUG("in for");
        Variable v($v, VOID);
        std::list<Type> type;
        if (isDefined(currentFile, @v.begin.line, $v, type)) {
            v = Variable($v, type.back());
            checkType(currentFile, @b.begin.line, "RANGE_BEGIN", type.back(), $b->getType());
            checkType(currentFile, @e.begin.line, "RANGE_END",  type.back(), $e->getType());
            checkType(currentFile, @s.begin.line, "RANGE_STEP", type.back(), $s->getType());
        }
        $$ = pb.createFor(v, $b, $e, $s, $ops);
        contextManager.leaveScope();
    }
    ;

while:
    WHILE '('booleanOperation[cond]')' {
        contextManager.enterScope();
    } block[ops] {
        DEBUG("in while");
        $$ = pb.createWhile($cond, $ops);
        contextManager.leaveScope();
    }
    ;
%%

void interpreter::Parser::error(const location_type& loc, const std::string& msg) {
    std::ostringstream oss;
    oss << currentFile << ":" << loc.begin.line << ": " << msg << "." << std::endl;
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
                  ap.first->getVariable()->getId(),
                  ap.first->getVariable()->getType(),
                  ap.first->getValue()->getType());
    }
}

/* Verify the types of all funcalls. To check the type, we have to verify the
 * types of all the parameters. The return type is not important here.
 */
void checkFuncalls() {
    // TODO: add the file location in the list
    for (auto fp : funcallsToCheck) {
        std::list<Type> funcallType = getTypes(fp.first->getParams());
        std::optional<Symbol> sym = contextManager.lookup(fp.first->getFunctionName());
        std::list<Type> expectedType;

        if (sym.has_value()) {
            // get the found return type (types of the parameters)
            std::list<Type> funcallType = getTypes(fp.first->getParams());
            fp.first->setType(expectedType.back());
            expectedType = sym.value().getType();
            expectedType.pop_back(); // remove the return type

            if (checkTypeError(expectedType, funcallType)) {
                errMgr.addFuncallTypeError(fp.second.first,
                                           fp.second.second,
                                           fp.first->getFunctionName(),
                                           expectedType, funcallType);
            }
        } else {
            // errMgr.addUndefinedSymbolError(fp.first->getFunctionName(), fp.second.first, fp.second.second);
        }
    }
}

void compile(std::string fileName, std::string outputName) {
    int parserOutput;
    int preprocessorOutput = 0;

    ProgramBuilder pb;
    Preprocessor pp(PREPROCESSOR_OUTPUT_FILE);

    currentFile = fileName;
    contextManager.enterScope(); // update the scope

    try {
        pp.process(fileName); // launch the preprocessor
    } catch (std::logic_error& e) {
        errMgr.addError(e.what());
        preprocessorOutput = 1;
    }

    // open and parse the file
    std::ifstream is("__main_pp.prog__", std::ios::in); // parse the preprocessed file
    interpreter::Scanner scanner{ is , std::cerr };
    interpreter::Parser parser{ &scanner, pb };
    parserOutput = parser.parse();
    checkFuncalls();
    checkAssignments();

    // loock for main
    std::optional<Symbol> sym = contextManager.lookup("main");
    if (0 == parserOutput && 0 == preprocessorOutput && !sym.has_value()) {
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
