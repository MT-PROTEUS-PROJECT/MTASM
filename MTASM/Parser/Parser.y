%language "c++"
%require "3.2"
%defines
%locations

%code requires {
    #include <variant>
    #include "../ASM/TypeDefs.h"
    namespace yy
    {
        struct Lexer;
    }
}

%define api.token.raw
%define api.value.type{ std::variant<std::string, Value> }

%parse-param{ Lexer & lexer }
%code{
    #include "../Lexer/Lexer.h"
    #include "../ASM/Register.h"
    #ifdef yylex
        #undef	yylex
    #endif
    #define	yylex lexer.yylex

    #include <vector>
    #include <memory>
    #include "../ASM/Input.h"
    #include "../ASM/Expressions.h"
    std::vector<std::unique_ptr<Input>> input;
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
    SUB
    MUL
    DIV
    NUM
    REG

%start block

%%
block:  expr { std::cout << "1\n"; }
|       error {};

expr:   aexpr {};

aexpr:  ADD aexprf { 
                        ArOp tmp(ArOp::Op::ADD, *(dynamic_cast<ArOpIn *>(input.back().get())));
                        input.pop_back();
                        std::cout << "MTEMU ADD: " << tmp.ToMtemuFmt() << std::endl;
                    }
|       SUB aexprf  {
                        ArOp tmp(ArOp::Op::SUB, *(dynamic_cast<ArOpIn *>(input.back().get())));
                        input.pop_back();
                        std::cout << "MTEMU SUB: " << tmp.ToMtemuFmt() << std::endl;
                    }
|       MUL aexprf  {
                        ArOp tmp(ArOp::Op::MUL, *(dynamic_cast<ArOpIn *>(input.back().get())));
                        input.pop_back();
                        std::cout << "MTEMU MUL: " << tmp.ToMtemuFmt() << std::endl;
                    }
|       DIV aexprf  {
                        ArOp tmp(ArOp::Op::DIV, *(dynamic_cast<ArOpIn *>(input.back().get())));
                        input.pop_back();
                        std::cout << "MTEMU DIV: " << tmp.ToMtemuFmt() << std::endl;
                    };

aexprf:	REG COMMA REG COMMA REG {
                                    Register r1(std::get<std::string>($1));
                                    Register r2(std::get<std::string>($3));
                                    Register r3(std::get<std::string>($5));
                                    if (r1 != r2 && r2 != r3 && !r1.isRQ() && !r2.isRQ() && !r3.isRQ())
                                        throw yy::parser::syntax_error(@$, "Использование 3 различных регистров общего назначения в арифиметических операциях не поддерживается!");
                                    if (r2.isRQ() && r3.isRQ())
                                        throw yy::parser::syntax_error(@$, "Регистр Q не может быть одновременно левым и правым операндом арифметической операции!");
                                    
                                    input.emplace_back(new ArOpIn(r1, r2, r3));
                                }
|       REG COMMA REG COMMA NUM { input.emplace_back(new ArOpIn(Register(std::get<std::string>($1)), Register(std::get<std::string>($3)), std::get<Value>($5))); }
|       REG COMMA NUM COMMA REG { input.emplace_back(new ArOpIn(Register(std::get<std::string>($1)), std::get<Value>($3), Register(std::get<std::string>($5)))); }
|       REG COMMA REG           {
                                    Register r1(std::get<std::string>($1));
                                    Register r2(std::get<std::string>($3));
                                    if (r1.isRQ() && r2.isRQ())
                                        throw yy::parser::syntax_error(@$, "Регистр Q не может быть одновременно левым и правым операндом арифметической операции!");
                                    input.emplace_back(new ArOpIn(r1, r2));
                                }
|       REG COMMA NUM           { input.emplace_back(new ArOpIn(Register(std::get<std::string>($1)), std::get<Value>($3))); }
|       NUM COMMA REG           { input.emplace_back(new ArOpIn(std::get<Value>($1), Register(std::get<std::string>($3)))); };

%%

void yy::parser::error(const location_type &loc, const std::string &err_message)
{
    std::cerr << "Error: " << err_message << " at " << loc << std::endl;
}
