%language "c++"
%require "3.2"
%defines
%locations

%code requires {
    #include <variant>
    #include "../ASM/TypeDefs.h"
    #include "../ASM/Expressions.h"
    namespace yy
    {
        struct Lexer;
    }
}

%define api.token.raw
%define api.value.type{ std::variant<std::string, Value, UnOp::Op> }

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
    std::vector<std::unique_ptr<Input>> input;
    yy::position::counter_type lineNumber = 1;
}

%define parse.trace
%define parse.error verbose
%define parse.lac full

%define api.token.prefix{TOKEN_}
%token
    END 0
    COMMA
    SEMICOLON
    COLON
    NUM
    REG
    LABEL
    ADD
    SUB
    MUL
    DIV
    OR
    AND
    XOR
    NXOR
    JNZ
    JMP
    JZ
    JF3
    JOVR
    JC4
    ;

%start block

%%
block:      block expr
|           error
|           %empty;

expr:       binexpr SEMICOLON
|           unexpr  SEMICOLON
|           LABEL   COLON;

binexpr:    ADD binexprf                {
                                            BinOp tmp(BinOp::Op::ADD, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                            input.pop_back();
                                            std::cout << lineNumber << '.' << @$.begin.column << " MTEMU ADD:\t" << tmp.ToMtemuFmt() << std::endl;
                                        }
|           SUB binexprf                {
                                            BinOp tmp(BinOp::Op::SUB, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                            input.pop_back();
                                            std::cout << lineNumber << '.' << @$.begin.column << " MTEMU SUB:\t" << tmp.ToMtemuFmt() << std::endl;
                                        }
|           MUL binexprf                {
                                            BinOp tmp(BinOp::Op::MUL, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                            input.pop_back();
                                            std::cout << lineNumber << '.' << @$.begin.column << " MTEMU MUL:\t" << tmp.ToMtemuFmt() << std::endl;
                                        }
|           DIV binexprf                {
                                            BinOp tmp(BinOp::Op::DIV, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                            input.pop_back();
                                            std::cout << lineNumber << '.' << @$.begin.column << " MTEMU DIV:\t" << tmp.ToMtemuFmt() << std::endl;
                                        }
|           OR binexprf                 {
                                            BinOp tmp(BinOp::Op::OR, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                            input.pop_back();
                                            std::cout << lineNumber << '.' << @$.begin.column << " MTEMU OR:\t" << tmp.ToMtemuFmt() << std::endl;
                                        }
|           AND binexprf                {
                                            BinOp tmp(BinOp::Op::AND, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                            input.pop_back();
                                            std::cout << lineNumber << '.' << @$.begin.column << " MTEMU AND:\t" << tmp.ToMtemuFmt() << std::endl;
                                        }
|           XOR binexprf                {
                                            BinOp tmp(BinOp::Op::XOR, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                            input.pop_back();
                                            std::cout << lineNumber << '.' << @$.begin.column << " MTEMU XOR:\t" << tmp.ToMtemuFmt() << std::endl;
                                        }
|           NXOR binexprf               {
                                            BinOp tmp(BinOp::Op::NXOR, *(dynamic_cast<BinOpIn *>(input.back().get())));
                                            input.pop_back();
                                            std::cout << lineNumber << '.' << @$.begin.column << " MTEMU NXOR:\t" << tmp.ToMtemuFmt() << std::endl;
                                        };

binexprf:	REG COMMA REG COMMA REG     {
                                            Register r1(std::get<std::string>($1));
                                            Register r2(std::get<std::string>($3));
                                            Register r3(std::get<std::string>($5));
                                            if (r1 != r2 && r2 != r3 && !r1.isRQ() && !r2.isRQ() && !r3.isRQ())
                                                throw yy::parser::syntax_error(@1, "Использование 3 различных регистров общего назначения в бинарных операциях не поддерживается!");
                                            if (r2.isRQ() && r3.isRQ())
                                                throw yy::parser::syntax_error(@1,"Регистр Q не может быть одновременно левым и правым операндом бинарной операции!");
                                    
                                            input.emplace_back(new BinOpIn(r1, r2, r3));
                                        }
|           REG COMMA REG COMMA NUM     { input.emplace_back(new BinOpIn(Register(std::get<std::string>($1)), Register(std::get<std::string>($3)), std::get<Value>($5))); }
|           REG COMMA NUM COMMA REG     { input.emplace_back(new BinOpIn(Register(std::get<std::string>($1)), std::get<Value>($3), Register(std::get<std::string>($5)))); }
|           REG COMMA REG               {
                                            Register r1(std::get<std::string>($1));
                                            Register r2(std::get<std::string>($3));
                                            if (r1.isRQ() && r2.isRQ())
                                                throw yy::parser::syntax_error(@1, "Регистр Q не может быть одновременно левым и правым операндом бинарной операции!");
                                            input.emplace_back(new BinOpIn(r1, r2));
                                        }
|           REG COMMA NUM               { input.emplace_back(new BinOpIn(Register(std::get<std::string>($1)), std::get<Value>($3))); }
|           NUM COMMA REG               { input.emplace_back(new BinOpIn(std::get<Value>($1), Register(std::get<std::string>($3)))); };

unexpr:     jumplbl LABEL               {
                                            UnOp tmp(std::get<UnOp::Op>($1), std::get<std::string>($2));
                                            std::cout << lineNumber << '.' << @$.begin.column << " MTEMU JUMP:\t" << tmp.ToMtemuFmt() << std::endl;
                                        };

jumplbl:    JNZ                         { $$.emplace<UnOp::Op>(UnOp::Op::JNZ); }
|           JMP                         { $$.emplace<UnOp::Op>(UnOp::Op::JMP); }
|           JZ                          { $$.emplace<UnOp::Op>(UnOp::Op::JZ); }
|           JF3                         { $$.emplace<UnOp::Op>(UnOp::Op::JF3); }
|           JOVR                        { $$.emplace<UnOp::Op>(UnOp::Op::JOVR); }
|           JC4                         { $$.emplace<UnOp::Op>(UnOp::Op::JC4); };

%%

void yy::parser::error(const location_type &loc, const std::string &err_message)
{
    std::cerr << "Стр: " << loc.begin.line + lineNumber - 1 << " стлб: " << loc.begin.column << ". Ошибка: " << err_message << std::endl;
}
