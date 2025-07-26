%{
#include <iostream>
#include <string>
#include <cstring>
#include <FlexLexer.h>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include "tools/messages.hpp"
#include "s3c.hpp"
#include "preprocessor/preprocessor.hpp"
#include "compiler/compiler.hpp"
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
%define api.namespace {parser}
%define api.value.type variant
%locations
%parse-param {Scanner* scanner} {s3c::State *state}

%code requires
{
    #include "tree/node.hpp"
    #include "s3c.hpp"
    namespace parser {
        class Scanner;
    }
}

%code
{
    #include "lexer.hpp"
    #include <memory>
    #include <functional>
    // #define yylex(x) scanner->lex(x)
    #define yylex(x, y) scanner->lex(x, y) // now we use yylval and yylloc
}

%token <long long>  INT
%token <double>     FLT
%token <char>       CHR
%token NIL
%token INTT FLTT CHRT STRT
%token CND OTW FOR WHL
%token COMMA OSQUAREB CSQUAREB
%token SHW IPT ADD SUB MUL DIV RNG MOV
%token EQL SUP INF SEQ IEQ AND LOR XOR NOT
%token <std::string> IDENTIFIER
%token <std::string> STRING
%token ERROR
%token RET
%token DCL
%token BGN END
%token TEXT
%token <std::string> PREPROCESSOR_LOCATION

%nterm <type::Type*> type
%nterm <type::Type*> returnTypeSpecifier
%nterm <node::Node*> parameterDeclaration
%nterm <node::Node*> value
%nterm <node::Node*> expression
%nterm <node::Node*> variable
%nterm <node::Node*> arithmeticOperation
%nterm <node::Node*> functionCall
%nterm <node::Node*> booleanOperation
%nterm <node::Block*> block
%nterm <node::Node*> cnd
%nterm <node::Node*> cndBase
%nterm <node::Node*> for
%nterm <node::Node*> whl

%start start

%%
start: program;

program: %empty | programUnit program ;

programUnit:
    functionDefinition { DEBUG("create new function" ); }
    | functionDeclaration { DEBUG("new function declaration" ); }
    | PREPROCESSOR_LOCATION { s3c::enter_file(state, $1); }
    ;

returnTypeSpecifier:
    type[rt] {
        $$ = $rt;
    }
    | NIL {
        $$ = type::create_nil_type();
    }
    ;

functionSignature:
    returnTypeSpecifier[rt] IDENTIFIER[name] {
        // TODO: function declaration
        if (!s3c::new_function_definition(state, $name, @name.begin.line)) {
            return 1;
        }
    } '('parameterDeclarationList')' {
        s3c::set_curr_function_type(state, $rt, @rt.begin.line);
    }
    ;

functionDeclaration:
    DCL functionSignature {
        s3c::add_function_declaration(state, @1.begin.line);
    }
    ;

functionDefinition:
    functionSignature block[body] {
        s3c::add_function_definition(state, state->curr_function.name,
                                     $body, @1.begin.line);
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
        $$ = s3c::new_argument_declaration(state, $2, $t, @2.begin.line);
    }
    | type[t] IDENTIFIER OSQUAREB INT[size] CSQUAREB {
        DEBUG("new param: " << $2);
        $$ = s3c::new_argument_declaration(state, $2,
            type::create_static_array_type($t, $size), @2.begin.line);
    }
    ;

parameterList:
    %empty
    | parameter
    | parameter COMMA parameterList
    ;

parameter:
    expression {
        s3c::save_function_call_argument(state, $1);
    }
    ;

type:
    INTT {
        $$ = type::create_primitive_type(type::PrimitiveType::Int);
    }
    | FLTT {
        $$ = type::create_primitive_type(type::PrimitiveType::Flt);
    }
    | CHRT {
        $$ = type::create_primitive_type(type::PrimitiveType::Chr);
    }
    | STRT {
        $$ =  type::create_primitive_type(type::PrimitiveType::Str);
    }
    ;

block:
    BGN {
        s3c::begin_block(state);
    } code END {
        DEBUG("new block");
        $$ = s3c::end_block(state);
    }
    ;

code:
    %empty
    | statement code
    | instruction code
    | RET expression[rs] {
        s3c::new_return_expr(state, $rs, @1.begin.line);
    }
    |
    RET {
        s3c::new_return_expr(state, nullptr, @1.begin.line);
    }
    ;

instruction:
    shw
    | ipt
    | variableDeclaration
    | assignment
    | functionCall { s3c::add_instruction(state, $1); }
    ;

ipt:
    IPT'('variable[c]')' {
        DEBUG("ipt var");
        s3c::add_instruction(state,
            node::create_builtin_function(
                location_create(state->curr_filename, @c.begin.line),
                node::BuiltinFunctionKind::Ipt, $c));
    }
    ;

shw:
    SHW'('expression[ic]')' {
        DEBUG("shw var");
        s3c::new_shw(state, $ic, @ic.begin.line);
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
        $$ = s3c::new_variable_reference(state, $1, @1.begin.line);
    }
    | IDENTIFIER OSQUAREB expression[index] CSQUAREB {
        DEBUG("using an array");
        $$ = s3c::new_index_expr(state, $1, @1.begin.line, $index);
    }
    ;

arithmeticOperation:
    ADD'(' expression[left] COMMA expression[right] ')' {
        DEBUG("addOP");
        $$ = s3c::new_arithmetic_operation(state, $left, $right,
            node::ArithmeticOperationKind::Add, @1.begin.line, "add");
    }
    | SUB'(' expression[left] COMMA expression[right] ')' {
        DEBUG("mnsOP");
        $$ = s3c::new_arithmetic_operation(state, $left, $right,
            node::ArithmeticOperationKind::Sub, @1.begin.line, "sub");
    }
    | MUL'(' expression[left] COMMA expression[right] ')' {
        DEBUG("tmsOP");
        $$ = s3c::new_arithmetic_operation(state, $left, $right,
            node::ArithmeticOperationKind::Mul, @1.begin.line, "mul");
    }
    | DIV'(' expression[left] COMMA expression[right] ')' {
        DEBUG("divOP");
        $$ = s3c::new_arithmetic_operation(state, $left, $right,
            node::ArithmeticOperationKind::Div, @1.begin.line, "div");
    }
    ;

booleanOperation:
    EQL'(' expression[lhs] COMMA expression[rhs] ')' {
        DEBUG("EqlOP");
        // TODO: type check
        $$ = node::create_boolean_operation(
            location_create(state->curr_filename, @1.begin.line),
            node::BooleanOperationKind::Eql, $lhs, $rhs);
    }
    | SUP'(' expression[lhs] COMMA expression[rhs] ')' {
        DEBUG("SupOP");
        // TODO: type check
        $$ = node::create_boolean_operation(
            location_create(state->curr_filename, @1.begin.line),
            node::BooleanOperationKind::Sup, $lhs, $rhs);
    }
    | INF'(' expression[lhs] COMMA expression[rhs] ')' {
        DEBUG("InfOP");
        // TODO: type check
        $$ = node::create_boolean_operation(
            location_create(state->curr_filename, @1.begin.line),
            node::BooleanOperationKind::Inf, $lhs, $rhs);
    }
    | SEQ'(' expression[lhs] COMMA expression[rhs] ')' {
        DEBUG("SeqOP");
        // TODO: type check
        $$ = node::create_boolean_operation(
            location_create(state->curr_filename, @1.begin.line),
            node::BooleanOperationKind::Seq, $lhs, $rhs);
    }
    | IEQ'(' expression[lhs] COMMA expression[rhs] ')' {
        DEBUG("IeqOP");
        // TODO: type check
        $$ = node::create_boolean_operation(
            location_create(state->curr_filename, @1.begin.line),
            node::BooleanOperationKind::Ieq, $lhs, $rhs);
    }
    | AND'('booleanOperation[lhs] COMMA booleanOperation[rhs]')' {
        DEBUG("AndOP");
        $$ = node::create_boolean_operation(
            location_create(state->curr_filename, @1.begin.line),
            node::BooleanOperationKind::And, $lhs, $rhs);
    }
    | LOR'('booleanOperation[lhs] COMMA booleanOperation[rhs]')' {
        DEBUG("LorOP");
        $$ = node::create_boolean_operation(
            location_create(state->curr_filename, @1.begin.line),
            node::BooleanOperationKind::Lor, $lhs, $rhs);
    }
    | XOR'('booleanOperation[lhs] COMMA booleanOperation[rhs]')' {
        DEBUG("XorOP");
        $$ = node::create_boolean_operation(
            location_create(state->curr_filename, @1.begin.line),
            node::BooleanOperationKind::Xor, $lhs, $rhs);
    }
    | NOT'('booleanOperation[op]')' {
        DEBUG("NotOP");
        $$ = node::create_boolean_operation(
            location_create(state->curr_filename, @1.begin.line),
            node::BooleanOperationKind::Not, $op, nullptr);
    }
    ;

functionCall:
    IDENTIFIER'(' {
        s3c::begin_new_funcall(state);
    }
    parameterList')' {
        DEBUG("new funcall: " << $1);
        $$ = s3c::new_function_call(state, $1, @1.begin.line);
    }
    ;

variableDeclaration:
    type[t] IDENTIFIER[name] {
        DEBUG("new declaration: " << $name);
        s3c::new_variable_declaration(state, $name, $t, @name.begin.line);
    }
    | type[t] IDENTIFIER[name] OSQUAREB INT[size] CSQUAREB {
        DEBUG("new array declaration: " << $2);
        s3c::new_variable_declaration(state, $name,
            type::create_static_array_type($t, $size), @name.begin.line);
    }
    ;

assignment:
    MOV'('variable[var] COMMA expression[expr]')' {
        DEBUG("new assignment");
        s3c::new_assignment(state, $var, $expr, @var.begin.line);
    }
    ;

value:
    INT {
        DEBUG("new int: " << $1);
        $$ = s3c::new_value<long>(state, (long)$1, @1.begin.line);
    }
    | FLT {
        DEBUG("new double: " << $1);
        $$ = s3c::new_value<double>(state, $1, @1.begin.line);
    }
    | CHR {
        DEBUG("new char: " << $1);
        $$ = s3c::new_value<char>(state, $1, @1.begin.line);
    }
    | STRING {
        DEBUG("new str: " << $1);
        $$ = s3c::new_value<std::string>(state, $1, @1.begin.line);
    }
    ;

statement:
    cnd {
        DEBUG("new if");
        s3c::add_instruction(state, $1);
    }
    | for {
        DEBUG("new for");
        s3c::add_instruction(state, $1);
    }
    | whl {
        DEBUG("new whl");
        s3c::add_instruction(state, $1);
    }
    ;

cnd:
    cndBase {
        $$ = $1;
    }
    | cndBase[cndb] OTW {
        DEBUG("els");
        s3c::enter_scope(state);
    } block[ops] {
        auto ifstmt = $cndb;
        // adding else block
        ifstmt->value.cnd_stmt->otw_block = $ops;
        $$ = ifstmt;
        s3c::leave_scope(state, $ops);
    }
    ;

cndBase:
    CND booleanOperation[cond] {
        s3c::enter_scope(state);
    } block[ops] {
        DEBUG("if");
        $$ = node::create_cnd_stmt(
            location_create(state->curr_filename, @1.begin.line),
            $cond, $ops);
        s3c::leave_scope(state, $ops);
    }
    ;

for:
    FOR IDENTIFIER[v] RNG'('expression[b] COMMA expression[e] COMMA expression[s]')' {
        s3c::enter_scope(state);
    } block[ops] {
        DEBUG("in for");
        $$ = s3c::new_for(state, $v, $b, $e, $s, $ops, @v.begin.line);
    }
    ;

whl:
    WHL booleanOperation[cond] {
        s3c::enter_scope(state);
    } block[ops] {
        DEBUG("in whl");
        $$ = node::create_whl_stmt(
            location_create(state->curr_filename, @1.begin.line),
            $cond, $ops);
        s3c::leave_scope(state, $ops);
    }
    ;
%%

void parser::Parser::error(const location_type& loc, const std::string& msg) {
    std::ostringstream oss;
    oss << state->curr_filename << ":" << loc.begin.line << ": " << msg << "." << std::endl;
    msg::error(oss.str());
}

/* Run interactive parser. It was used during the beginning of the project. */
void cli() {
    s3c::State *state = s3c::state_create();
    parser::Scanner scanner{ std::cin, std::cerr };
    parser::Parser parser{ &scanner, state };
    s3c::enter_scope(state);
    parser.parse();
    // TODO s3c.errorsManager().report();
    // if (!s3c.errorsManager().getErrors()) {
    //     TODO s3c.programBuilder().display();
    // }
    delete state;
}

/* add execution rights to the result file */
void makeExecutable(std::string file) {
    std::filesystem::permissions(file,
            std::filesystem::perms::owner_exec
            | std::filesystem::perms::group_exec
            | std::filesystem::perms::others_exec,
            std::filesystem::perm_options::add);
}

void compile(std::string filename, std::string outputName) {
    int parserOutput;
    int preprocessorErrorStatus = 0;

    s3c::State *state = s3c::state_create();
    Preprocessor pp(PREPROCESSOR_OUTPUT_FILE);

    // s3c::enter_file(state, filename);
    // s3c::enter_scope(state);

    try {
        pp.process(filename); // launch the preprocessor
    } catch (std::logic_error& e) {
        msg::error(e.what());
        preprocessorErrorStatus = 1;
    }

    // open and parse the file
    std::ifstream is(PREPROCESSOR_OUTPUT_FILE, std::ios::in); // parse the preprocessed file
    parser::Scanner scanner{ is , std::cerr };
    parser::Parser parser{ &scanner, state };
    parserOutput = parser.parse();
    s3c::post_process(state);

    // look for main
    if (!try_verify_main_type(state)) {
        std::cout << "wrong main type" << std::endl;
    }

    // if no errors, transpile the file
    // TODO: use the status from the state
    // if (!s3c.errorsManager().getErrors()) {
        compiler::compile(filename, compiler::Arch::X86_64,
                          compiler::Platform::GNULinux,
                          Program{state->program, state->scopes.global});
        // TODO: check compile status before running ld
        std::string ld_cmd = "ld -o " + outputName + " " + compiler::object_filename(filename) + " -lc -dynamic-linker /lib64/ld-linux-x86-64.so.2";
        std::cout << "running: " << ld_cmd << std::endl;
        system(ld_cmd.c_str());
    // }

    // remove the preprocessor output file
    if (REMOVE_PREPROCESSOR_FILE) {
        std::filesystem::remove(PREPROCESSOR_OUTPUT_FILE);
    }
    delete state;
}

int main(int argc, char **argv) {
    if (argc == 2) {
        compile(argv[1], "bin"); // TODO: add an option to choose the name of the created script
    } else { // launch the interpreter for debugging
        cli();
    }
}
