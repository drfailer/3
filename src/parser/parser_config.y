%{
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <sstream>
#include "s3c.hpp"
#include "tools/messages.hpp"
#define YYLOCATION_PRINT   location_print
#define YYDEBUG 1
#define DBG_PARS 0
#if DBG_PARS == 1
#define DEBUG(A) std::cout << A << std::endl
#else
#define DEBUG(A)
#endif
%}
%language "c++"
%defines "parser.hpp"
%output "parser.cpp"

%define api.parser.class {Parser}
%define api.namespace {parser}
%define api.value.type variant
%locations
%parse-param {Scanner *scanner} {s3c::State *state}

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
%token COMMA SEMI OSQUAREB CSQUAREB
%token SHW IPT ADD SUB MUL DIV MOV
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
%nterm <node::Node*> assignment
%nterm <node::Node*> expression
%nterm <node::Node*> variable
%nterm <node::Node*> arithmeticOperation
%nterm <node::Node*> functionCall
%nterm <node::Node*> booleanOperation
%nterm <node::Node*> block
%nterm <node::Node*> cnd
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
        s3c::new_function_definition(state, $name, @name.begin.line);
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
    | assignment { s3c::add_instruction(state, $1); }
    | functionCall { s3c::add_instruction(state, $1); }
    ;

ipt:
    IPT'('variable[c]')' {
        DEBUG("ipt var");
        s3c::add_instruction(state,
            new_node(&state->node_pool, location_create(state->curr_filename, @c.begin.line),
                node::NodeKind::BuiltinFunction, .builtin_function = node::BuiltinFunction{
                        .kind = node::BuiltinFunctionKind::Ipt,
                        .argument = $c,
                    }
                ));
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
        $$ = new_node(&state->node_pool, location_create(state->curr_filename, @1.begin.line),
                      node::NodeKind::BooleanOperation, .boolean_operation = node::BooleanOperation{
                          .kind = node::BooleanOperationKind::Eql,
                          .lhs = $lhs,
                          .rhs = $rhs,
                      });
    }
    | SUP'(' expression[lhs] COMMA expression[rhs] ')' {
        DEBUG("SupOP");
        $$ = new_node(&state->node_pool, location_create(state->curr_filename, @1.begin.line),
                      node::NodeKind::BooleanOperation, .boolean_operation = node::BooleanOperation{
                          .kind = node::BooleanOperationKind::Sup,
                          .lhs = $lhs,
                          .rhs = $rhs,
                      });
    }
    | INF'(' expression[lhs] COMMA expression[rhs] ')' {
        DEBUG("InfOP");
        $$ = new_node(&state->node_pool, location_create(state->curr_filename, @1.begin.line),
                      node::NodeKind::BooleanOperation, .boolean_operation = node::BooleanOperation{
                          .kind = node::BooleanOperationKind::Inf,
                          .lhs = $lhs,
                          .rhs = $rhs,
                      });
    }
    | SEQ'(' expression[lhs] COMMA expression[rhs] ')' {
        DEBUG("SeqOP");
        $$ = new_node(&state->node_pool, location_create(state->curr_filename, @1.begin.line),
                      node::NodeKind::BooleanOperation, .boolean_operation = node::BooleanOperation{
                          .kind = node::BooleanOperationKind::Seq,
                          .lhs = $lhs,
                          .rhs = $rhs,
                      });
    }
    | IEQ'(' expression[lhs] COMMA expression[rhs] ')' {
        DEBUG("IeqOP");
        $$ = new_node(&state->node_pool, location_create(state->curr_filename, @1.begin.line),
                      node::NodeKind::BooleanOperation, .boolean_operation = node::BooleanOperation{
                          .kind = node::BooleanOperationKind::Ieq,
                          .lhs = $lhs,
                          .rhs = $rhs,
                      });
    }
    | AND'('booleanOperation[lhs] COMMA booleanOperation[rhs]')' {
        DEBUG("AndOP");
        $$ = new_node(&state->node_pool, location_create(state->curr_filename, @1.begin.line),
                      node::NodeKind::BooleanOperation, .boolean_operation = node::BooleanOperation{
                          .kind = node::BooleanOperationKind::And,
                          .lhs = $lhs,
                          .rhs = $rhs,
                      });
    }
    | LOR'('booleanOperation[lhs] COMMA booleanOperation[rhs]')' {
        DEBUG("LorOP");
        $$ = new_node(&state->node_pool, location_create(state->curr_filename, @1.begin.line),
                      node::NodeKind::BooleanOperation, .boolean_operation = node::BooleanOperation{
                          .kind = node::BooleanOperationKind::Lor,
                          .lhs = $lhs,
                          .rhs = $rhs,
                      });
    }
    | XOR'('booleanOperation[lhs] COMMA booleanOperation[rhs]')' {
        DEBUG("XorOP");
        $$ = new_node(&state->node_pool, location_create(state->curr_filename, @1.begin.line),
                      node::NodeKind::BooleanOperation, .boolean_operation = node::BooleanOperation{
                          .kind = node::BooleanOperationKind::Xor,
                          .lhs = $lhs,
                          .rhs = $rhs,
                      });
    }
    | NOT'('booleanOperation[op]')' {
        DEBUG("NotOP");
        $$ = new_node(&state->node_pool, location_create(state->curr_filename, @1.begin.line),
                      node::NodeKind::BooleanOperation, .boolean_operation = node::BooleanOperation{
                          .kind = node::BooleanOperationKind::Not,
                          .lhs = $op,
                          .rhs = nullptr,
                      });
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
        $$ = s3c::new_assignment(state, $var, $expr, @var.begin.line);
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
    CND {
        s3c::enter_scope(state);
        s3c::begin_block(state);
    } booleanOperation[cond] BGN {
        s3c::new_cnd(state, $cond, @1.begin.line);
    } cndContent END {
        $$ = s3c::end_cnd(state);
    }
    ;

cndContent:
    code optOtw
    ;

otw:
    OTW {
        s3c::new_otw(state);
    }
    ;

optOtw:
    %empty
    | otw booleanOperation[cond] {
        s3c::new_otw_cnd(state, $cond, @1.begin.line);
    } cndContent
    | otw code
    ;

for:
    FOR assignment[b] SEMI booleanOperation[e] SEMI expression[s] {
        s3c::enter_scope(state);
    } block[ops] {
        DEBUG("in for");
        $$ = s3c::new_for(state, $b, $e, $s, $ops, @1.begin.line);
    }
    ;

whl:
    WHL booleanOperation[cond] {
        s3c::enter_scope(state);
    } block[ops] {
        DEBUG("in whl");
        $$ = new_node(&state->node_pool, location_create(state->curr_filename, @1.begin.line),
                      node::NodeKind::WhlStmt, .whl_stmt = node::WhlStmt{
                          .condition = $cond,
                          .block = $ops,
                      });
        s3c::leave_scope(state, $ops);
    }
    ;
%%

void parser::Parser::error(const location_type& loc, const std::string& msg) {
    std::ostringstream oss;
    oss << state->curr_filename << ":" << loc.begin.line << ": " << msg << "." << std::endl;
    msg::error(oss.str());
}
