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
%parse-param {Scanner &scanner} {State *state}

%code requires
{
    #include "ast.hpp"
    #include "s3c.hpp"
    namespace parser {
        class Scanner;
    }
    struct CndContent {
        Ast *block;
        Ast *otw;
    };
}

%code
{
    #include "lexer.hpp"
    #include <memory>
    #include <functional>
    // #define yylex(x) scanner.lex(x)
    #define yylex(x, y) scanner.lex(x, y) // now we use yylval and yylloc
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

%nterm <TypeSpecifier> type
%nterm <Ast*> functionSignature
%nterm <std::vector<Ast*>> parameterDeclarationList
%nterm <std::vector<Ast*>> parameterList
%nterm <Ast*> value
%nterm <Ast*> assignment
%nterm <Ast*> expression
%nterm <Ast*> variable
%nterm <Ast*> variableDefinition
%nterm <Ast*> arithmeticOperation
%nterm <Ast*> functionCall
%nterm <Ast*> booleanOperation
%nterm <Ast*> block
%nterm <Ast*> cnd
%nterm <Ast*> for
%nterm <Ast*> whl
%nterm <Ast*> ret
%nterm <Ast*> instruction
%nterm <Ast*> builtinFunctionCall
%nterm <Ast*> controlStructure
%nterm <std::vector<Ast*>> code
%nterm <std::vector<Ast*>> instructions
%nterm <CndContent> cndContent
%nterm <Ast*> optOtw

%start start

%%
start: program;

program: %empty | programUnit program ;

programUnit:
    functionDefinition { DEBUG("create new function" ); }
    | functionDeclaration { DEBUG("new function declaration" ); }
    | PREPROCESSOR_LOCATION { enter_file(state, $1); }
    ;

functionSignature:
    type[rt] IDENTIFIER[name] '('parameterDeclarationList[args]')' {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @name.begin.line),
            AstKind::Function,
            .function = {
                .return_type_specifier = $rt,
                .name = string_create($name, state->allocator),
                .arguments = array_create_from_std_vector($args, state->allocator),
                .body = nullptr,
            }
        );
    }
    ;

functionDeclaration:
    DCL functionSignature[function] {
        add_function(state, $function);
    }
    ;

functionDefinition:
    functionSignature[function] block[body] {
        $function->data.function.body = $body;
        add_function(state, $function);
    }
    ;

parameterDeclarationList:
    %empty { $$ = std::vector<Ast*>(); }
    | variableDefinition { $$ = std::vector<Ast*>({$1}); }
    | parameterDeclarationList[args] COMMA variableDefinition[arg] {
        $args.push_back($arg);
        $$ = $args;
    }
    ;

parameterList:
    %empty { $$ = std::vector<Ast*>(); }
    | expression { $$ = std::vector<Ast*>({$1}); }
    | parameterList[args] COMMA expression[arg] {
        $args.push_back($arg);
        $$ = $args;
    }
    ;

type:
    NIL { $$ = TypeSpecifier{TypeSpecifierKind::Nil, {}, 0}; }
    | INTT { $$ = TypeSpecifier{TypeSpecifierKind::Int, {}, 0}; }
    | FLTT { $$ = TypeSpecifier{TypeSpecifierKind::Flt, {}, 0}; }
    | CHRT { $$ = TypeSpecifier{TypeSpecifierKind::Chr, {}, 0}; }
    | STRT { $$ = TypeSpecifier{TypeSpecifierKind::Str, {}, 0}; }
    ;

block:
    BGN code END {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::Block,
            .block = { array_create_from_std_vector($code, state->allocator) }
        );
    }
    ;

code:
    instructions[ops] ret {
        $ops.push_back($ret);
        $$ = $ops;
    }
    | instructions
    ;

instructions:
    %empty { $$ = std::vector<Ast*>(); }
    | instructions instruction {
        $1.push_back($instruction);
        $$ = $1;
    }
    ;

instruction:
    builtinFunctionCall
    | variableDefinition
    | assignment
    | functionCall
    | controlStructure
    ;

ret:
    RET expression[expr] {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::RetStmt,
            .ret_stmt = { $expr }
        );
    }
    | RET {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::RetStmt,
            .ret_stmt = { nullptr }
        );
    }

builtinFunctionCall:
    IPT '(' variable[var] ')' {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::BuiltinFunction,
            .builtin_function = {
                .kind = BuiltinFunctionKind::Ipt,
                .argument = $var,
            }
        );
    }
    | SHW '(' expression[expr] ')' {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::BuiltinFunction,
            .builtin_function = {
               .kind = BuiltinFunctionKind::Shw,
               .argument = $expr,
            }
        );
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
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::VariableReference,
            .variable_reference = {
                .name = string_create($1, state->allocator),
            }
        );
    }
    | variable[var] OSQUAREB expression[index] CSQUAREB {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @var.begin.line),
            AstKind::IndexExpression,
            .index_expression = {
                .element = $var,
                .index = $index,
            }
        );
    }
    ;

arithmeticOperation:
    ADD'(' expression[lhs] COMMA expression[rhs] ')' {
        $$ = new_arithmetic_operation(state, $lhs, $rhs, ArithmeticOperationKind::Add, @1.begin.line);
    }
    | SUB'(' expression[lhs] COMMA expression[rhs] ')' {
        $$ = new_arithmetic_operation(state, $lhs, $rhs, ArithmeticOperationKind::Sub, @1.begin.line);
    }
    | MUL'(' expression[lhs] COMMA expression[rhs] ')' {
        $$ = new_arithmetic_operation(state, $lhs, $rhs, ArithmeticOperationKind::Mul, @1.begin.line);
    }
    | DIV'(' expression[lhs] COMMA expression[rhs] ')' {
        $$ = new_arithmetic_operation(state, $lhs, $rhs, ArithmeticOperationKind::Div, @1.begin.line);
    }
    ;

booleanOperation:
    EQL'(' expression[lhs] COMMA expression[rhs] ')' {
        $$ = new_boolean_operation(state, $lhs, $rhs, BooleanOperationKind::Eql, @1.begin.line);
    }
    | SUP'(' expression[lhs] COMMA expression[rhs] ')' {
        $$ = new_boolean_operation(state, $lhs, $rhs, BooleanOperationKind::Sup, @1.begin.line);
    }
    | INF'(' expression[lhs] COMMA expression[rhs] ')' {
        $$ = new_boolean_operation(state, $lhs, $rhs, BooleanOperationKind::Inf, @1.begin.line);
    }
    | SEQ'(' expression[lhs] COMMA expression[rhs] ')' {
        $$ = new_boolean_operation(state, $lhs, $rhs, BooleanOperationKind::Seq, @1.begin.line);
    }
    | IEQ'(' expression[lhs] COMMA expression[rhs] ')' {
        $$ = new_boolean_operation(state, $lhs, $rhs, BooleanOperationKind::Ieq, @1.begin.line);
    }
    | AND'('booleanOperation[lhs] COMMA booleanOperation[rhs]')' {
        $$ = new_boolean_operation(state, $lhs, $rhs, BooleanOperationKind::And, @1.begin.line);
    }
    | LOR'('booleanOperation[lhs] COMMA booleanOperation[rhs]')' {
        $$ = new_boolean_operation(state, $lhs, $rhs, BooleanOperationKind::Lor, @1.begin.line);
    }
    | XOR'('booleanOperation[lhs] COMMA booleanOperation[rhs]')' {
        $$ = new_boolean_operation(state, $lhs, $rhs, BooleanOperationKind::Xor, @1.begin.line);
    }
    | NOT'('booleanOperation[op]')' {
        $$ = new_boolean_operation(state, $op, nullptr, BooleanOperationKind::Not, @1.begin.line);
    }
    ;

functionCall:
    IDENTIFIER[name]'('parameterList[args]')' {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @name.begin.line),
            AstKind::FunctionCall,
            .function_call = {
                .name = string_create($name, state->allocator),
                .arguments = array_create_from_std_vector($args, state->allocator),
            }
        );
    }
    ;

variableDefinition:
    type[t] IDENTIFIER[name] {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @name.begin.line),
            AstKind::VariableDefinition,
            .variable_definition = {
                .type_specifier = $t,
                .name = string_create($name, state->allocator),
            }
        );
    }
    | type[t] IDENTIFIER[name] OSQUAREB INT[size] CSQUAREB {
        $t.size = $size;
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @name.begin.line),
            AstKind::VariableDefinition,
            .variable_definition = {
                .type_specifier = $t,
                .name = string_create($name, state->allocator),
            }
        );
    }
    ;

assignment:
    MOV'('variable[var] COMMA expression[expr]')' {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::Assignment,
            .assignment = Assignment{ $var, $expr }
        );
    }
    ;

value:
    INT { $$ = new_value<long>(state, (long)$1, @1.begin.line); }
    | FLT { $$ = new_value<double>(state, $1, @1.begin.line); }
    | CHR { $$ = new_value<char>(state, $1, @1.begin.line); }
    | STRING { $$ = new_value<std::string>(state, $1, @1.begin.line); }
    ;

controlStructure: cnd | for | whl;

cnd:
    CND  booleanOperation[cond] BGN cndContent[cc] END {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::CndStmt,
            .cnd_stmt = {
                .condition = $cond,
                .block = $cc.block,
                .otw = $cc.otw,
            }
        );
    }
    ;

cndContent:
    code optOtw {
        auto block = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::Block,
            .block = { array_create_from_std_vector($1, state->allocator) }
        );
        $$ = CndContent{
            .block = block,
            .otw = $2,
        };
    }
    ;

optOtw:
    %empty { $$ = nullptr; }
    | OTW booleanOperation[cond] cndContent[cc] {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::CndStmt,
            .cnd_stmt = {
                .condition = $cond,
                .block = $cc.block,
                .otw = $cc.otw,
            }
        );
    }
    | OTW code {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::Block,
            .block { array_create_from_std_vector($2, state->allocator) }
        );
    }
    ;

for:
    FOR assignment[i] SEMI booleanOperation[c] SEMI expression[s] block[ops] {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::ForStmt,
            .for_stmt = {
                .init = $i,
                .condition = $c,
                .step = new_ast(
                    &state->ast_pool,
                    location_create(state->curr_filename, @s.begin.line),
                    AstKind::Assignment,
                    .assignment = {
                        .target = $i->data.assignment.target,
                        .value = $s,
                    }
                ),
                .block = $ops,
            }
        );
    }
    ;

whl:
    WHL booleanOperation[cond] block[ops] {
        $$ = new_ast(
            &state->ast_pool,
            location_create(state->curr_filename, @1.begin.line),
            AstKind::WhlStmt,
            .whl_stmt = {
                .condition = $cond,
                .block = $ops,
            }
        );
    }
    ;
%%

void parser::Parser::error(const location_type& loc, const std::string& msg) {
    std::ostringstream oss;
    oss << state->curr_filename << ":" << loc.begin.line << ": " << msg << "." << std::endl;
    msg::error(oss.str());
}
