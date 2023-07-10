%top{
	#include <cstdint>
}

%{
	#include "Parser.tab.hh"
	#include "Lexer.h"
	#include "../ASM/TypeDefs.h"
%}

%option c++
%option noyywrap
%option nodefault
%option nounistd
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
{DIGIT1}{DIGIT}*	    { yylval->emplace<Value>(std::stoul(yytext)); return yy::parser::token::TOKEN_NUM; }
"R"{REG}				{ yylval->emplace<std::string>(yytext); return yy::parser::token::TOKEN_REG; }
"ADD"					return yy::parser::token::TOKEN_ADD;
","						return yy::parser::token::TOKEN_COMMA;
";"						return yy::parser::token::TOKEN_SEMICOLON;
.						throw yy::parser::syntax_error(*yyloc, "invalid character: " + std::string(YYText()));
<<EOF>>					return yy::parser::token::TOKEN_END;

%%
