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
#include "symtable/context_manager.hpp"
#include "tools/program_builder.hpp"
#include "tools/errors_manager.hpp"
#include "tools/s3c.hpp"
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
%parse-param {Scanner* scanner} {S3C &s3c}

%code requires
{
    #include "ast/ast.hpp"
    #include "tools/s3c.hpp"
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

    std::list<std::pair<std::shared_ptr<FunctionCall>, std::pair<std::string, int>>> funcallsToCheck = {};
    std::list<std::pair<std::shared_ptr<Assignment>, std::pair<std::string, int>>> assignmentsToCheck = {};
    std::list<std::pair<std::string, std::function<bool(type_system::type_t)>>> expressionsToCheck = {};
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

%nterm <type_system::type_t> type
%nterm <type_system::type_t> returnTypeSpecifier
%nterm <std::shared_ptr<Value>> value
%nterm <std::shared_ptr<TypedNode>> expression
%nterm <std::shared_ptr<Variable>> variable
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
        s3c.programBuilder().currFileName($1);
    }
    ;

returnTypeSpecifier:
    type[rt] {
        $$ = $rt;
    }
    | NIL {
        $$ = type_system::make_type<type_system::Primitive>(type_system::NIL);
    }
    ;

functionDefinition:
    returnTypeSpecifier[rt] IDENTIFIER[name] {
        if (!s3c.newFunctionDefinition($name, @name.begin.line)) {
            return 1;
        }
    } '('parameterDeclarationList')' {
        s3c.setFunctionType($rt);
    } block[ops] {
        s3c.endFunctionDefintion($name, $rt, $ops);
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
        s3c.newParameterDeclaration($2, $t);
    }
    | type[t] IDENTIFIER OSQUAREB INT[size] CSQUAREB {
        DEBUG("new param: " << $2);
        s3c.newArrayParameterDeclaration($2, $t, $size);
    }
    ;

parameterList:
    %empty
    | parameter
    | parameter COMMA parameterList
    ;

parameter:
    expression {
        s3c.programBuilder().pushFuncallParam($1);
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
        s3c.programBuilder().beginBlock();
    } code END {
        DEBUG("new block");
        $$ = s3c.programBuilder().endBlock();
    }
    ;

code:
    %empty
    | statement code
    | instruction code
    | RET expression[rs] {
        s3c.newReturnExpression($rs, @1.begin.line);
    }
    ;

instruction:
    shw
    | ipt
    | variableDeclaration
    | assignment
    | functionCall { s3c.programBuilder().pushBlock($1); }
    ;

ipt:
    IPT'('variable[c]')' {
        DEBUG("ipt var");
        s3c.programBuilder().pushBlock(std::make_shared<Ipt>($c));
    }
    ;

shw:
    SHW'('expression[ic]')' {
        DEBUG("shw var");
        s3c.newShw($ic);
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
        $$ = s3c.newVariable($1, @1.begin.line);
    }
    | IDENTIFIER OSQUAREB expression[index] CSQUAREB {
        DEBUG("using an array");
        $$ = s3c.newArrayVariable($1, @1.begin.line, $index);
    }
    ;

arithmeticOperation:
    ADD'(' expression[left] COMMA expression[right] ')' {
        DEBUG("addOP");
        $$ = s3c.newArithmeticOperator<AddOP>($left, $right, @1.begin.line, "add");
    }
    | MNS'(' expression[left] COMMA expression[right] ')' {
        DEBUG("mnsOP");
        $$ = s3c.newArithmeticOperator<MnsOP>($left, $right, @1.begin.line, "mns");
    }
    | TMS'(' expression[left] COMMA expression[right] ')' {
        DEBUG("tmsOP");
        $$ = s3c.newArithmeticOperator<TmsOP>($left, $right, @1.begin.line, "tms");
    }
    | DIV'(' expression[left] COMMA expression[right] ')' {
        DEBUG("divOP");
        $$ = s3c.newArithmeticOperator<DivOP>($left, $right, @1.begin.line, "div");
    }
    ;

booleanOperation:
    EQL'(' expression[left] COMMA expression[right] ')' {
        DEBUG("EqlOP");
        // TODO: type check
        $$ = std::make_shared<EqlOP>($left, $right);
    }
    | SUP'(' expression[left] COMMA expression[right] ')' {
        DEBUG("SupOP");
        // TODO: type check
        $$ = std::make_shared<SupOP>($left, $right);
    }
    | INF'(' expression[left] COMMA expression[right] ')' {
        DEBUG("InfOP");
        // TODO: type check
        $$ = std::make_shared<InfOP>($left, $right);
    }
    | SEQ'(' expression[left] COMMA expression[right] ')' {
        DEBUG("SeqOP");
        // TODO: type check
        $$ = std::make_shared<SeqOP>($left, $right);
    }
    | IEQ'(' expression[left] COMMA expression[right] ')' {
        DEBUG("IeqOP");
        // TODO: type check
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
        s3c.programBuilder().newFuncall($1);
    }
    parameterList')' {
        DEBUG("new funcall: " << $1);
        $$ = s3c.newFunctionCall($1, @1.begin.line, funcallsToCheck);
    }
    ;

variableDeclaration:
    type[t] IDENTIFIER[name] {
        DEBUG("new declaration: " << $name);
        s3c.newVariableDeclaration($name, $t, @name.begin.line);
    }
    | type[t] IDENTIFIER[name] OSQUAREB INT[size] CSQUAREB {
        DEBUG("new array declaration: " << $2);
        s3c.newArrayDeclaration($name, $t, $size, @name.begin.line);
    }
    ;

assignment:
    SET'('variable[var] COMMA expression[expr]')' {
        DEBUG("new assignment");
        s3c.newAssignment($var, $expr, @var.begin.line, assignmentsToCheck);
    }
    ;

value:
    INT {
        DEBUG("new int: " << $1);
        $$ = s3c.newInt($1);
    }
    | FLT {
        DEBUG("new double: " << $1);
        $$ = s3c.newFlt($1);
    }
    | CHR {
        DEBUG("new char: " << $1);
        $$ = s3c.newChr($1);
    }
    | STRING {
        DEBUG("new char: " << $1);
        auto strValue = s3c.newStr($1, @1.begin.line);
        if (!strValue)
            return 1;
        $$ = strValue;
    }
    ;

statement:
    cnd {
        DEBUG("new if");
        s3c.programBuilder().pushBlock($1);
    }
    | for {
        DEBUG("new for");
        s3c.programBuilder().pushBlock($1);
    }
    | whl {
        DEBUG("new whl");
        s3c.programBuilder().pushBlock($1);
    }
    ;

cnd:
    cndBase {
        $$ = $1;
    }
    | cndBase[cndb] ELS {
        DEBUG("els");
        s3c.contextManager().enterScope();
    } block[ops] {
        std::shared_ptr<Cnd> ifstmt = $cndb;
        // adding else block
        ifstmt->elseBlock = $ops;
        $$ = ifstmt;
        s3c.contextManager().leaveScope();
    }
    ;

cndBase:
    CND booleanOperation[cond] {
        s3c.contextManager().enterScope();
    } block[ops] {
        DEBUG("if");
        $$ = s3c.programBuilder().createCnd($cond, $ops);
        s3c.contextManager().leaveScope();
    }
    ;

for:
    FOR IDENTIFIER[v] RNG'('expression[b] COMMA expression[e] COMMA expression[s]')' {
        s3c.contextManager().enterScope();
    } block[ops] {
        DEBUG("in for");
        $$ = s3c.newFor($v, $b, $e, $s, $ops, @v.begin.line);
    }
    ;

whl:
    WHL booleanOperation[cond] {
        s3c.contextManager().enterScope();
    } block[ops] {
        DEBUG("in whl");
        $$ = s3c.programBuilder().createWhl($cond, $ops);
        s3c.contextManager().leaveScope();
    }
    ;
%%

void interpreter::Parser::error(const location_type& loc, const std::string& msg) {
    std::ostringstream oss;
    oss << s3c.programBuilder().currFileName() << ":" << loc.begin.line << ": " << msg << "." << std::endl;
    s3c.errorsManager().addError(oss.str());
}

/* Run interactive parser. It was used during the beginning of the project. */
void cli() {
    S3C s3c;
    interpreter::Scanner scanner{ std::cin, std::cerr };
    interpreter::Parser parser{ &scanner, s3c };
    s3c.contextManager().enterScope();
    parser.parse();
    s3c.errorsManager().report();
    if (!s3c.errorsManager().getErrors()) {
        s3c.programBuilder().display();
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
 // TODO: should be moved elsewhere
void checkAssignments(S3C &s3c) {
    for (auto ap : assignmentsToCheck) {
        checkType(s3c, ap.second.first, ap.second.second,
                  ap.first->variable->id,
                  ap.first->variable->type,
                  ap.first->value->type->getEvaluatedType());
    }
}

// todo: we shouldn't need this
type_system::types_t getTypes(std::list<std::shared_ptr<TypedNode>> const &nodes) {
    type_system::types_t parametersTypes = {};

    std::transform(nodes.cbegin(), nodes.cend(),
                   std::back_insert_iterator<type_system::types_t>(parametersTypes),
                   [](auto elt) { return elt->type; });
    return parametersTypes;
}

/* Verify the types of all funcalls. To check the type, we have to verify the
 * types of all the parameters. The return type is not important here.
 */
// TODO: will be moved elsewhere
void checkFuncalls(S3C &s3c) {
    // std::list<std::pair<std::shared_ptr<FunctionCall>,
    //                     std::pair<std::string, int>>> funcallsToCheck;
    for (auto fp : funcallsToCheck) {
        std::optional<Symbol> sym = s3c.contextManager().lookup(fp.first->functionName);

        if (sym.has_value()) {
            // get the found return type (types of the parameters)
            type_system::types_t foundArgumentsTypes = getTypes(fp.first->parameters);
            type_system::types_t expectedArgumentsTypes =
                std::static_pointer_cast<type_system::Function>(
                                            sym.value().getType())->argumentsTypes;

            if (!checkParametersTypes(s3c, expectedArgumentsTypes, foundArgumentsTypes)) {
                s3c.errorsManager().addFuncallTypeError(fp.second.first,
                                           fp.second.second,
                                           fp.first->functionName,
                                           expectedArgumentsTypes, foundArgumentsTypes);
            }
        } else {
            // todo
            // s3c.errorsManager().addUndefinedSymbolError(fp.first->functionName(), fp.second.first, fp.second.second);
        }
    }
}

void compile(std::string fileName, std::string outputName) {
    int parserOutput;
    int preprocessorErrorStatus = 0;

    S3C s3c;
    Preprocessor pp(PREPROCESSOR_OUTPUT_FILE);

    s3c.programBuilder().currFileName(fileName);
    s3c.contextManager().enterScope(); // update the scope

    try {
        pp.process(fileName); // launch the preprocessor
    } catch (std::logic_error& e) {
        s3c.errorsManager().addError(e.what());
        preprocessorErrorStatus = 1;
    }

    // open and parse the file
    std::ifstream is(PREPROCESSOR_OUTPUT_FILE, std::ios::in); // parse the preprocessed file
    interpreter::Scanner scanner{ is , std::cerr };
    interpreter::Parser parser{ &scanner, s3c };
    parserOutput = parser.parse();
    checkFuncalls(s3c);
    checkAssignments(s3c);

    // look for main
    std::optional<Symbol> sym = s3c.contextManager().lookup("main");
    if (0 == parserOutput && 0 == preprocessorErrorStatus && !sym.has_value()) {
        s3c.errorsManager().addNoEntryPointError();
    }
    // report errors and warnings
    s3c.errorsManager().report();

    // if no errors, transpile the file
    if (!s3c.errorsManager().getErrors()) {
        std::ofstream fs(outputName);
        s3c.programBuilder().program()->compile(fs);
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
