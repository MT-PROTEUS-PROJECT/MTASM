%top{
    #include <cstdint>
}

%{
    #include "../ASM/TypeDefs.h"
    #include "Parser.tab.hh"
    #include "Lexer.h"
    namespace details
    {
        extern yy::position::counter_type lineNumber;
    }
%}

%option c++
%option noyywrap
%option nodefault
%option nounistd
%option yylineno
%option yyclass = "yy::Lexer"

WS [ \t\r\n]
REG Q|[0-9]|1[0-5]
DIGIT [0-9]
DIGIT1 [1-9]
LABEL [a-zA-Z_][a-zA-Z_0-9]*

%%

{WS}+                   
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
"CALL"                  return yy::parser::token::TOKEN_CALL;
"CLNZ"                  return yy::parser::token::TOKEN_CLNZ;
"JSP"                   return yy::parser::token::TOKEN_JSP;
"JSNZ"                  return yy::parser::token::TOKEN_JSNZ;
"JSNC4"                 return yy::parser::token::TOKEN_JSNC4;
"RET"                   return yy::parser::token::TOKEN_RET;
"PUSH"                  return yy::parser::token::TOKEN_PUSH;
"POP"                   return yy::parser::token::TOKEN_POP;
"END_LDM"               return yy::parser::token::TOKEN_END_LDM;
"LSL"                   return yy::parser::token::TOKEN_LSL;
"LSR"                   return yy::parser::token::TOKEN_LSR;
"CSL"                   return yy::parser::token::TOKEN_CSL;
"CSR"                   return yy::parser::token::TOKEN_CSR;
"CDSL"                  return yy::parser::token::TOKEN_CDSL;
"CDSR"                  return yy::parser::token::TOKEN_CDSR;
"ADSL"                  return yy::parser::token::TOKEN_ADSL;
"ADSR"                  return yy::parser::token::TOKEN_ADSR;
"LSLQ"                  return yy::parser::token::TOKEN_LSLQ;
"LSRQ"                  return yy::parser::token::TOKEN_LSRQ;
"CSLQ"                  return yy::parser::token::TOKEN_CSLQ;
"CSRQ"                  return yy::parser::token::TOKEN_CSRQ;
"CDSLQ"                 return yy::parser::token::TOKEN_CDSLQ;
"CDSRQ"                 return yy::parser::token::TOKEN_CDSRQ;
"ADSLQ"                 return yy::parser::token::TOKEN_ADSLQ;
"ADSRQ"                 return yy::parser::token::TOKEN_ADSRQ;
"SET"                   return yy::parser::token::TOKEN_SET;
{LABEL}                 { yylval->emplace<std::string>(yytext); return yy::parser::token::TOKEN_LABEL; }
","						return yy::parser::token::TOKEN_COMMA;
";"                     return yy::parser::token::TOKEN_SEMICOLON;
":"                     return yy::parser::token::TOKEN_COLON;
.						throw yy::parser::syntax_error(*yyloc, "invalid character: " + std::string(YYText(), YYLeng()));
<<EOF>>                 return yy::parser::token::TOKEN_END;

%%
