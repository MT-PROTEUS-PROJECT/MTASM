%top{
    #include <cstdint>
}

%{
    #include "Parser.tab.hh"
    #include "Lexer.h"
    #include "../ASM/TypeDefs.h"

    extern yy::position::counter_type lineNumber;
%}

%option c++
%option noyywrap
%option nodefault
%option nounistd
%option yyclass = "yy::Lexer"
%x comment

WS [ \t\r\n]
REG Q|[0-9]|1[0-5]
DIGIT [0-9]
DIGIT1 [1-9]

%%

%{
    yyloc->step();
%}

"//"					BEGIN(comment);
"\n"					{ ++lineNumber; yyloc->step(); }
{WS}+                   yyloc->step();
"R"{REG}                { yylval->emplace<std::string>(yytext); return yy::parser::token::TOKEN_REG; }
{DIGIT1}{DIGIT}*        { yylval->emplace<Value>(std::stoul(yytext)); return yy::parser::token::TOKEN_NUM; }
"ADD"					return yy::parser::token::TOKEN_ADD;
"SUB"					return yy::parser::token::TOKEN_SUB;
"MUL"					return yy::parser::token::TOKEN_MUL;
"DIV"					return yy::parser::token::TOKEN_DIV;
"OR"					return yy::parser::token::TOKEN_OR;
"AND"					return yy::parser::token::TOKEN_AND;
"XOR"					return yy::parser::token::TOKEN_XOR;
"NXOR"					return yy::parser::token::TOKEN_NXOR;
","						return yy::parser::token::TOKEN_COMMA;
";"                     return yy::parser::token::TOKEN_SEMICOLON;
.						throw yy::parser::syntax_error(*yyloc, "invalid character: " + std::string(YYText()));
<<EOF>>                 return yy::parser::token::TOKEN_END;

%%
