%language "c++"
%require "3.2"
%defines
%locations

%code requires {
    #include <variant>
    #include "../ASM/TypeDefs.h"
    #include "../ASM/Expressions.h"
    #include "../ASM/Exceptions.h"
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
    #include "../ASM/Label.h"
    #ifdef yylex
        #undef	yylex
    #endif
    #define	yylex lexer.yylex

    #include <vector>
    #include <queue>
    #include <memory>
    #include <unordered_map>
    #include <string>
    #include "../ASM/Input.h"
    #include "../ASM/Publisher.h"

    namespace details
    {
        std::vector<std::unique_ptr<Input>> input;
        std::queue<std::shared_ptr<Expression>> exprs;
        std::unordered_map<std::shared_ptr<Label>, int32_t, Label::PtrHash, Label::PtrEqual> labels;
        std::string lastLabel;
        ::yy::position::counter_type lineNumber = 1;
    }
    
    void flushExprs();
}

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

%destructor                             {
                                            for (const auto &[lbl, pos] : details::labels)
                                            {
                                                if (pos == -1)
                                                    throw yy::parser::syntax_error(@$, "Метка '" + lbl->GetStr() + "' не найдена!");
                                            }
                                        } END

%start block

%%

block:      block expr
|           error                       { ++details::lineNumber; }
|           %empty                      { ++details::lineNumber; }
;

expr:       binexpr SEMICOLON           { flushExprs(); }
|           unexpr  SEMICOLON           { flushExprs(); }
|           LABEL   COLON               {
                                            auto lbl = std::make_shared<Label>(std::get<std::string>($1));
                                            lbl->IncrAddr(Publisher::GetInstance()->Size());
                                            if (details::labels.contains(lbl))
                                            {
                                                if (details::labels[lbl] != -1)
                                                    throw yy::parser::syntax_error(@1, "Метка '" + lbl->GetStr() +  "' уже существует!");
                                                else
                                                {
                                                    details::labels[lbl] = details::lineNumber;
                                                    auto node = details::labels.extract(lbl);
                                                    node.key()->SetAddr(lbl->GetAddr());
                                                    details::labels.insert(std::move(node));
                                                }
                                            }
                                            else
                                            {
                                                auto [it, ok] = details::labels.emplace(std::move(lbl), details::lineNumber);
                                                if (!ok)
                                                    throw InternalCompilerError("Не удалось добавить метку в хэш-таблицу");
                                            }
                                        }
;

binexpr:    ADD binexprf                {
                                            details::exprs.push(std::make_shared<BinOp>(BinOp::Op::ADD, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                            details::input.pop_back();
                                            std::cout << details::lineNumber << '.' << @$.begin.column << "\tMTEMU ADD:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                        }
|           SUB binexprf                {
                                            details::exprs.push(std::make_shared<BinOp>(BinOp::Op::SUB, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                            details::input.pop_back();
                                            std::cout << details::lineNumber << '.' << @$.begin.column << "\tMTEMU SUB:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                        }
|           MUL binexprf                {
                                            details::exprs.push(std::make_shared<BinOp>(BinOp::Op::MUL, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                            details::input.pop_back();
                                            std::cout << details::lineNumber << '.' << @$.begin.column << "\tMTEMU MUL:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                        }
|           DIV binexprf                {
                                            details::exprs.push(std::make_shared<BinOp>(BinOp::Op::DIV, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                            details::input.pop_back();
                                            std::cout << details::lineNumber << '.' << @$.begin.column << "\tMTEMU DIV:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                        }
|           OR binexprf                 {
                                            details::exprs.push(std::make_shared<BinOp>(BinOp::Op::OR, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                            details::input.pop_back();
                                            std::cout << details::lineNumber << '.' << @$.begin.column << "\tMTEMU OR:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                        }
|           AND binexprf                {
                                            details::exprs.push(std::make_shared<BinOp>(BinOp::Op::AND, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                            details::input.pop_back();
                                            std::cout << details::lineNumber << '.' << @$.begin.column << "\tMTEMU AND:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                        }
|           XOR binexprf                {
                                            details::exprs.emplace(std::make_shared<BinOp>(BinOp::Op::XOR, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                            details::input.pop_back();
                                            std::cout << details::lineNumber << '.' << @$.begin.column << "\tMTEMU XOR:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                        }
|           NXOR binexprf               {
                                            details::exprs.push(std::make_shared<BinOp>(BinOp::Op::NXOR, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                            details::input.pop_back();
                                            std::cout << details::lineNumber << '.' << @$.begin.column << "\tMTEMU NXOR:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                        }
;

binexprf:	REG COMMA REG COMMA REG     {
                                            Register r1(std::get<std::string>($1));
                                            Register r2(std::get<std::string>($3));
                                            Register r3(std::get<std::string>($5));
                                            if (r1 != r2 && r2 != r3 && !r1.isRQ() && !r2.isRQ() && !r3.isRQ())
                                                throw yy::parser::syntax_error(@1, "Использование 3 различных регистров общего назначения в бинарных операциях не поддерживается!");
                                            if (r2.isRQ() && r3.isRQ())
                                                throw yy::parser::syntax_error(@1,"Регистр Q не может быть одновременно левым и правым операндом бинарной операции!");
                                    
                                            details::input.push_back(std::make_unique<BinOpIn>(r1, r2, r3));
                                        }
|           REG COMMA REG COMMA NUM     { details::input.push_back(std::make_unique<BinOpIn>(Register(std::get<std::string>($1)), Register(std::get<std::string>($3)), std::get<Value>($5))); }
|           REG COMMA NUM COMMA REG     { details::input.push_back(std::make_unique<BinOpIn>(Register(std::get<std::string>($1)), std::get<Value>($3), Register(std::get<std::string>($5)))); }
|           REG COMMA REG               {
                                            Register r1(std::get<std::string>($1));
                                            Register r2(std::get<std::string>($3));
                                            if (r1.isRQ() && r2.isRQ())
                                                throw yy::parser::syntax_error(@1, "Регистр Q не может быть одновременно левым и правым операндом бинарной операции!");
                                            details::input.push_back(std::make_unique<BinOpIn>(r1, r2));
                                        }
|           REG COMMA NUM               { details::input.push_back(std::make_unique<BinOpIn>(Register(std::get<std::string>($1)), std::get<Value>($3))); }
|           NUM COMMA REG               { details::input.push_back(std::make_unique<BinOpIn>(std::get<Value>($1), Register(std::get<std::string>($3)))); }
;

unexpr:     jumplbl LABEL               {
                                            auto lbl = std::make_shared<Label>(std::get<std::string>($2));
                                            if (!details::labels.contains(lbl))
                                            {
                                                details::labels.emplace(lbl, -1);
                                                details::exprs.push(std::make_shared<UnOp>(std::get<UnOp::Op>($1), lbl));
                                            }
                                            else
                                            {
                                                auto node = details::labels.extract(lbl);
                                                details::exprs.push(std::make_shared<UnOp>(std::get<UnOp::Op>($1), node.key()));
                                            }
                                            std::cout << details::lineNumber << '.' << @$.begin.column << "\tMTEMU JUMP:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                        }
;

jumplbl:    JNZ                         { $$.emplace<UnOp::Op>(UnOp::Op::JNZ); }
|           JMP                         { $$.emplace<UnOp::Op>(UnOp::Op::JMP); }
|           JZ                          { $$.emplace<UnOp::Op>(UnOp::Op::JZ); }
|           JF3                         { $$.emplace<UnOp::Op>(UnOp::Op::JF3); }
|           JOVR                        { $$.emplace<UnOp::Op>(UnOp::Op::JOVR); }
|           JC4                         { $$.emplace<UnOp::Op>(UnOp::Op::JC4); }
;

%%

void yy::parser::error(const location_type &loc, const std::string &err_message)
{
    std::cerr << "Стр: " << loc.begin.line + details::lineNumber - 1 << " стлб: " << loc.begin.column << ". Ошибка: " << err_message << std::endl;
}

void flushExprs()
{
    while (!details::exprs.empty())
    {
        Publisher::GetInstance()->Push(details::exprs.front());
        details::exprs.pop();
    }
}
