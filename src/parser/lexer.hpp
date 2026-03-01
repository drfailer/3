#ifndef LEXER_HPP
#define LEXER_HPP
#include "parser.hpp"
#if !defined (yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

namespace parser {
class Scanner : public yyFlexLexer {
  public:
    Scanner(std::istream &arg_yyin, std::ostream &arg_yyout)
        : yyFlexLexer(arg_yyin, arg_yyout) {}
    Scanner(std::istream *arg_yyin = nullptr, std::ostream *arg_yyout = nullptr)
        : yyFlexLexer(arg_yyin, arg_yyout) {}
    int lex(Parser::semantic_type *yylval, Parser::location_type *yylloc);
};
} // namespace parser

#endif
