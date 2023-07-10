%language "c++"
%require "3.2"
%defines
%locations

%code requires {
	#include <variant>
	namespace yy
	{
		struct Lexer;
	}
}

%define api.token.raw
%define api.value.type {std::variant<std::string, uint32_t>}

%parse-param{ Lexer &lexer }
%code{
#include "../Lexer/Lexer.h"
#ifdef yylex
	#undef	yylex
#endif
#define	yylex lexer.yylex
}

%define parse.trace
%define parse.error verbose
%define parse.lac full

%define api.token.prefix{TOKEN_}
%token
	END 0
	COMMA
	SEMICOLON
	ADD
	NUM
	REG

%start block

%%
block : expr	{ std::cout << "1\n"; }
|		error	{ };

expr : aexpr { std::cout << "2\n"; };

aexpr : ADD aexprf { std::cout << "3\n"; };

aexprf :	REG COMMA REG COMMA REG { std::cout << "4 " << std::get<std::string>($1) << "\n"; }
|			REG COMMA REG COMMA NUM { }
|			REG COMMA NUM COMMA REG {}
|			REG COMMA REG			{}
|			REG COMMA NUM			{}
|			NUM COMMA REG			{};

%%

void yy::parser::error(const location_type &loc, const std::string &err_message)
{
	std::cerr << "Error: " << err_message << " at " << loc << std::endl;
}
