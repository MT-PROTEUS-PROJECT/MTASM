%top{
    #include <cstdint>
}

%{
    #include "Parser.tab.hh"
    #include "Lexer.h"
    #include "../ASM/TypeDefs.h"
    namespace details
    {
        extern yy::position::counter_type lineNumber;
    }
%}

%option c++
%option noyywrap
%option nodefault
%option nounistd
%option yyclass = "yy::Lexer"

WS [ \t\r\n]
REG Q|[0-9]|1[0-5]
DIGIT [0-9]
DIGIT1 [1-9]
LABEL [a-zA-Z_][a-zA-Z_0-9]*

%%

%{
    yyloc->step();
%}

"\n"					{ ++details::lineNumber; yyloc->step(); }
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
"JNZ"                   return yy::parser::token::TOKEN_JNZ;
"JMP"                   return yy::parser::token::TOKEN_JMP;
"JZ"                    return yy::parser::token::TOKEN_JZ;
"JF3"                   return yy::parser::token::TOKEN_JF3;
"JOVR"                  return yy::parser::token::TOKEN_JOVR;
"JC4"                   return yy::parser::token::TOKEN_JC4;
{LABEL}                 { yylval->emplace<std::string>(yytext); return yy::parser::token::TOKEN_LABEL; }
","						return yy::parser::token::TOKEN_COMMA;
";"                     return yy::parser::token::TOKEN_SEMICOLON;
":"                     return yy::parser::token::TOKEN_COLON;
.						throw yy::parser::syntax_error(*yyloc, "invalid character: " + std::string(YYText()));
<<EOF>>                 return yy::parser::token::TOKEN_END;

%%
