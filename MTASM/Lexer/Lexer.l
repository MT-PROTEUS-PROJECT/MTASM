%option c++
%option noyywrap nodefault
%option nounistd

%{
	#include "Parser.tab.hh"
	#include "Lexer.h"
	#include <cstdint>
%}

%option yyclass = "yy::Lexer"
%x comment

WS [ \t\r\n]
REG [Q|[0-9]|1[0-5]]
DIGIT [0-9]
DIGIT1 [1-9]

%%

%{
	yyloc->step();
%}

"//"					BEGIN(comment);
{WS}+					yyloc->step();
"#"{DIGIT1}{DIGIT}*		return yy::pareser::token::TOKEN_NUM;
"R"{REG}				return yy::parser::token::TOKEN_REGISTER;
"ADD"					return yy::parser::token::TOKEN_ADD;

.						throw yy::parser::syntax_error(*yyloc, "invalid character: " + std::string(YYText()));
<<EOF>>					return yy::parser::token::TOKEN_END;

%%
