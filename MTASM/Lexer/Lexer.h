#pragma once

#ifndef yyFlexLexerOnce
#include <FlexLexer.h>
#endif

#ifdef YY_DECL
#undef	YY_DECL
#endif

#define	YY_DECL	\
	yy::parser::token::yytokentype yy::Lexer::yylex(yy::parser::semantic_type *yylval, \
			yy::location *yyloc)


namespace yy
{
    class Lexer final : public yyFlexLexer
    {
    public:
        using FlexLexer::yylex;
        yy::parser::token::yytokentype yylex(yy::parser::semantic_type *yylval,
            yy::location *yyloc);
        Lexer(std::istream &in) : yyFlexLexer(in, std::cout)
        {}

        yy::location getlocation() const { return yy::location(nullptr, lineno(), 1); }
    };
}
