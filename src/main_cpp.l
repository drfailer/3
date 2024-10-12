%{
#include <string>
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
comma ,
blanks [\S\s \t\n]
string \"(\\.|[^"\\])*\"
comment \~\~\~.*$
file -->.+-{digit}+\n

%%
int          { DEBUG("L_nbrt"); return Parser::token::INTT; }
flt          { DEBUG("L_fltt"); return Parser::token::FLTT; }
chr          { DEBUG("L_chrt"); return Parser::token::CHRT; }
cnd          { DEBUG("L_cnd"); return Parser::token::CND; }
els          { DEBUG("L_els"); return Parser::token::ELS; }
whl          { DEBUG("L_whl"); return Parser::token::WHL; }
for          { DEBUG("L_for"); return Parser::token::FOR; }
shw          { DEBUG("L_shw"); return Parser::token::SHW; }
ipt          { DEBUG("L_ipt"); return Parser::token::IPT; }
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
lor          { DEBUG("L_lor"); return Parser::token::LOR; }
xor          { DEBUG("L_xor"); return Parser::token::XOR; }
not          { DEBUG("L_not"); return Parser::token::NOT; }
set          { DEBUG("L_set"); return Parser::token::SET; }
rng          { DEBUG("L_rng"); return Parser::token::RNG; }
ret          { DEBUG("L_ret"); return Parser::token::RET; }
bgn          { DEBUG("L_bng"); return Parser::token::BGN; }
end          { DEBUG("L_bng"); return Parser::token::END; }
"["          { DEBUG("L_osquareb"); return Parser::token::OSQUAREB; }
"]"          { DEBUG("L_osquareb"); return Parser::token::CSQUAREB; }

{file} {
    DEBUG("L_file");
    std::string filename = yytext + 3;
    int lineNumber = 0;
    int multiplier = 1;

    filename.pop_back();
    // parse the line number at the end of the file name
    while (filename.back() != '-') {
        lineNumber += multiplier * (filename.back() - '0');
        multiplier *= 10;
        filename.pop_back();
    }
    filename.pop_back();
    // reinitialize the location (note: the file is manage manually in the
    // parser, we don't use flex locations).
    yylloc->initialize(nullptr);
    yylloc->lines(lineNumber);
    yylloc->step();
    yylval->build<std::string>(filename);
    return Parser::token::PREPROCESSOR_LOCATION;
}

nil {
    DEBUG("L_nil");
    yylval->build<char>(yytext[1]);
    return Parser::token::NIL;
}

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
    DEBUG("L_flt");
    yylval->build<double>((double) std::atof(yytext));
    return Parser::token::FLT;
}

{char} {
    DEBUG("L_chr");
    yylval->build<char>(yytext[1]);
    return Parser::token::CHR;
}

{string} {
    DEBUG("L_string");
    yylval->build<std::string>(yytext);
    return Parser::token::STRING;
}

{comma}      { DEBUG("L_comma"); return Parser::token::COMMA; }
{comment}    { DEBUG("L_comment"); } // comment => do nothing
\n           { yylloc->lines(yyleng); yylloc->step(); }
{blanks}
.            { return yytext[0]; return Parser::token::TEXT; }
%%

int yyFlexLexer::yylex() {
    throw std::runtime_error("Bad call to yyFlexLexer::yylex()");
}
