%language "c++"
%require "3.2"
%defines
%locations

%code requires {
	namespace yy {
		class Driver;
	}
}

%define parse.trace
%define parse.error verbose
%define parse.lac full

%define api.token.prefix{TOKEN_}
%token
	END 0
	REGISTER
	NUM
	ADD


%destructor{ delete $$; } NUM REGISTER blocks

%start program
%%
program : blocks END { std::cout << "pr" << std::endl; };
blocks :	blocks {};
| %empty {};
%%
