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
%nterm <type_system::type> returnTypeSpecifier
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
        // TODO: we need a none type as if the function is not defined, we
        // cannot get the return type (should be verified afterward)
        std::shared_ptr<FunctionCall> funcall;
        if (std::optional<Symbol> symbol = s3c.contextManager().lookup($1)) {
            funcall =
            s3c.programBuilder().createFuncall(type_system::make_type<type_system::Primitive>(symbol.value().getType()->getEvaluatedType()));
        } else {
            funcall = s3c.programBuilder().createFuncall();
        }
        std::pair<std::string, int> position = std::make_pair(s3c.programBuilder().currFileName(), @1.begin.line);
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
        if (std::optional<Symbol> symbol = s3c.contextManager().lookup($name)) {
            s3c.errorsManager().addMultipleDefinitionError(s3c.programBuilder().currFileName(), @name.begin.line, $name);
        }
        s3c.contextManager().newSymbol($2, $t, LOCAL_VAR);
        s3c.programBuilder().pushBlock(std::make_shared<Declaration>(Variable($2, $t)));
    }
    | type[t] IDENTIFIER[name] OSQUAREB INT[size] CSQUAREB {
        DEBUG("new array declaration: " << $2);
        // redefinitions are not allowed:
        if (std::optional<Symbol> symbol = s3c.contextManager().lookup($name)) {
            s3c.errorsManager().addMultipleDefinitionError(s3c.programBuilder().currFileName(), @name.begin.line, $name);
        }
        s3c.contextManager().newSymbol($name, $t, $size, LOCAL_ARRAY);
        s3c.programBuilder().pushBlock(std::make_shared<ArrayDeclaration>($name, $size, $t));
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
            auto position = std::make_pair(s3c.programBuilder().currFileName(), @c.begin.line);
            assignmentsToCheck.push_back(std::pair(newAssignment, position));
        } else {
            checkType(s3c, s3c.programBuilder().currFileName(), @c.begin.line, v->id, $c->type, icType->getEvaluatedType());
        }
        s3c.programBuilder().pushBlock(newAssignment);
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
            s3c.errorsManager().addLiteralStringOverflowError(s3c.programBuilder().currFileName(), @1.begin.line);
            return 1;
        }
        memcpy(v._str, $1.c_str(), $1.size());
        $$ = std::make_shared<Value>(v, type_system::make_type<type_system::StaticArray>(type_system::CHR, $1.size()));
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
        Variable v($v, type_system::make_type<type_system::Primitive>(type_system::NIL));
        type_system::type type;
        if (isDefined(s3c, s3c.programBuilder().currFileName(), @v.begin.line, $v, type)) {
            v = Variable($v, type);
            checkType(s3c, s3c.programBuilder().currFileName(), @b.begin.line, "RANGE_BEGIN", type, $b->type->getEvaluatedType());
            checkType(s3c, s3c.programBuilder().currFileName(), @e.begin.line, "RANGE_END",  type, $e->type->getEvaluatedType());
            checkType(s3c, s3c.programBuilder().currFileName(), @s.begin.line, "RANGE_STEP", type, $s->type->getEvaluatedType());
        }
        $$ = s3c.programBuilder().createFor(v, $b, $e, $s, $ops);
        s3c.contextManager().leaveScope();
    }
    ;

whl:
    WHL '('booleanOperation[cond]')' {
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
// TODO: will be moved elsewhere
void checkFuncalls(S3C &s3c) {
    // std::list<std::pair<std::shared_ptr<FunctionCall>,
    //                     std::pair<std::string, int>>> funcallsToCheck;
    for (auto fp : funcallsToCheck) {
        std::optional<Symbol> sym = s3c.contextManager().lookup(fp.first->functionName);

        if (sym.has_value()) {
            // get the found return type (types of the parameters)
            type_system::types foundArgumentsTypes = getTypes(fp.first->parameters);
            type_system::types expectedArgumentsTypes =
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
    std::ifstream is("__main_pp.prog__", std::ios::in); // parse the preprocessed file
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
