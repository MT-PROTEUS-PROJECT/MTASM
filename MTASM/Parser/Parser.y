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
    uint64_t lineNumber = 1;
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
    OR
    AND
    XOR
    NXOR;

%start block

%%
block:      block expr { ++lineNumber; }
|           error
|           %empty;

expr:       binexpr;

binexpr:    ADD binexprf    {
                                BinOp tmp(BinOp::Op::ADD, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                input.pop_back();
                                std::cout << lineNumber << " MTEMU ADD:\t" << tmp.ToMtemuFmt() << std::endl;
                            }
|           SUB binexprf    {
                                BinOp tmp(BinOp::Op::SUB, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                input.pop_back();
                                std::cout << lineNumber << " MTEMU SUB:\t" << tmp.ToMtemuFmt() << std::endl;
                            }
|           MUL binexprf    {
                                BinOp tmp(BinOp::Op::MUL, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                input.pop_back();
                                std::cout << lineNumber << " MTEMU MUL:\t" << tmp.ToMtemuFmt() << std::endl;
                            }
|           DIV binexprf    {
                                BinOp tmp(BinOp::Op::DIV, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                input.pop_back();
                                std::cout << lineNumber << " MTEMU DIV:\t" << tmp.ToMtemuFmt() << std::endl;
                            }
|           OR binexprf     {
                                BinOp tmp(BinOp::Op::OR, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                input.pop_back();
                                std::cout << lineNumber << " MTEMU OR:\t" << tmp.ToMtemuFmt() << std::endl;
                            }
|           AND binexprf    {
                                BinOp tmp(BinOp::Op::AND, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                input.pop_back();
                                std::cout << lineNumber << " MTEMU AND:\t" << tmp.ToMtemuFmt() << std::endl;
                            }
|           XOR binexprf    {
                                BinOp tmp(BinOp::Op::XOR, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                input.pop_back();
                                std::cout << lineNumber << " MTEMU XOR:\t" << tmp.ToMtemuFmt() << std::endl;
                            }
|           NXOR binexprf   {
                                BinOp tmp(BinOp::Op::NXOR, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                input.pop_back();
                                std::cout << lineNumber << " MTEMU NXOR:\t" << tmp.ToMtemuFmt() << std::endl;
                            };

binexprf:	REG COMMA REG COMMA REG     {
                                            Register r1(std::get<std::string>($1));
                                            Register r2(std::get<std::string>($3));
                                            Register r3(std::get<std::string>($5));
                                            if (r1 != r2 && r2 != r3 && !r1.isRQ() && !r2.isRQ() && !r3.isRQ())
                                                throw yy::parser::syntax_error(@$, "Использование 3 различных регистров общего назначения в арифиметических операциях не поддерживается!");
                                            if (r2.isRQ() && r3.isRQ())
                                                throw yy::parser::syntax_error(@$, "Регистр Q не может быть одновременно левым и правым операндом арифметической операции!");
                                    
                                            input.emplace_back(new BinOpIn(r1, r2, r3));
                                        }
|           REG COMMA REG COMMA NUM { input.emplace_back(new BinOpIn(Register(std::get<std::string>($1)), Register(std::get<std::string>($3)), std::get<Value>($5))); }
|           REG COMMA NUM COMMA REG { input.emplace_back(new BinOpIn(Register(std::get<std::string>($1)), std::get<Value>($3), Register(std::get<std::string>($5)))); }
|           REG COMMA REG           {
                                        Register r1(std::get<std::string>($1));
                                        Register r2(std::get<std::string>($3));
                                        if (r1.isRQ() && r2.isRQ())
                                            throw yy::parser::syntax_error(@$, "Регистр Q не может быть одновременно левым и правым операндом арифметической операции!");
                                        input.emplace_back(new BinOpIn(r1, r2));
                                    }
|           REG COMMA NUM           { input.emplace_back(new BinOpIn(Register(std::get<std::string>($1)), std::get<Value>($3))); }
|           NUM COMMA REG           { input.emplace_back(new BinOpIn(std::get<Value>($1), Register(std::get<std::string>($3)))); };

%%

void yy::parser::error(const location_type &loc, const std::string &err_message)
{
    std::cerr << "Error: " << err_message << " at " << loc << std::endl;
}
