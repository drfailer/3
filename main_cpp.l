%{
#include "parser.hpp"
#include "lexer.hpp"
#define YY_DECL int interpreter::Scanner::lex(Parser::semantic_type *yylval, Parser::location_type *yylloc)
#define DBG_LEX 0
#if DBG_LEX == 1
#define DEBUG(A) std::cout << A << std::endl
#else
#define DEBUG(A)
#endif
%}

%option c++ interactive noyywrap yylineno nodefault outfile="lexer.cpp"

alpha [a-zA-Z]
digit [0-9]
int [+-]?{digit}+
float [+-]?{digit}+\.{digit}+
char '{alpha}'
identifier [a-z]({alpha}|{digit}|_)*
semi ;
comma ,
equal =
blanks [\S\s \t\n]
str \"(\\.|[^"\\])*\"
comment #.*$

%%
int          { DEBUG("L_nbrt"); return Parser::token::INTT; }
flt          { DEBUG("L_fltt"); return Parser::token::FLTT; }
chr          { DEBUG("L_chrt"); return Parser::token::CHRT; }
fn           { DEBUG("L_fn"); return Parser::token::FN; }
if           { DEBUG("L_if"); return Parser::token::IF; }
else         { DEBUG("L_else"); return Parser::token::ELSE; }
while        { DEBUG("L_while"); return Parser::token::WHILE; }
for          { DEBUG("L_for"); return Parser::token::FOR; }
in           { DEBUG("L_in"); return Parser::token::IN; }
print        { DEBUG("L_print"); return Parser::token::PRINT; }
read         { DEBUG("L_read"); return Parser::token::READ; }
add          { DEBUG("L_add"); return Parser::token::ADD; }
mns          { DEBUG("L_mns"); return Parser::token::MNS; }
tms          { DEBUG("L_tms"); return Parser::token::TMS; }
div          { DEBUG("L_div"); return Parser::token::DIV; }
eql          { DEBUG("L_eql"); return Parser::token::EQL; }
sup          { DEBUG("L_sup"); return Parser::token::SUP; }
inf          { DEBUG("L_inf"); return Parser::token::INF; }
seq          { DEBUG("L_seq"); return Parser::token::SEQ; }
ieq          { DEBUG("L_ieq"); return Parser::token::IEQ; }
and          { DEBUG("L_and"); return Parser::token::AND; }
or           { DEBUG("L_or"); return Parser::token::OR; }
xor          { DEBUG("L_xor"); return Parser::token::XOR; }
not          { DEBUG("L_not"); return Parser::token::NOT; }
set          { DEBUG("L_set"); return Parser::token::SET; }
include      { DEBUG("L_include"); return Parser::token::INCLUDE; }
range        { DEBUG("L_range"); return Parser::token::RANGE; }
->           { DEBUG("L_arrow"); return Parser::token::ARROW; }
return       { DEBUG("L_return"); return Parser::token::RETURN; }
"["          { DEBUG("L_osquareb"); return Parser::token::OSQUAREB; }
"]"          { DEBUG("L_osquareb"); return Parser::token::CSQUAREB; }

{identifier} {
  DEBUG("L_id");
  yylval->build<std::string>(yytext);
  return Parser::token::IDENTIFIER;
}

{int} {
  DEBUG("L_int: ");
  yylval->build<long long>((long long) std::atol(yytext));
  return Parser::token::INT;
}

{float} {
  DEBUG("L_float");
  yylval->build<double>((double) std::atof(yytext));
  return Parser::token::FLOAT;
}

{char} {
  DEBUG("L_char");
  yylval->build<char>(yytext[1]);
  return Parser::token::CHAR;
}

{str} {
  DEBUG("L_string");
  yylval->build<std::string>(yytext);
  return Parser::token::STRING;
}

{semi}       { DEBUG("L_semi"); return Parser::token::SEMI; }
{comma}      { DEBUG("L_comma"); return Parser::token::COMMA; }
{comment}    { DEBUG("L_comment"); } // comment => do nothing
\n           { yylloc->lines(yyleng); yylloc->step(); }
{blanks}
.            { return yytext[0]; return Parser::token::TEXT; }
%%

int yyFlexLexer::yylex() {
    throw std::runtime_error("Bad call to yyFlexLexer::yylex()");
}
