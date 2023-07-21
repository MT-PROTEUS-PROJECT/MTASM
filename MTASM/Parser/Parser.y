%language "c++"
%require "3.2"
%defines
%locations

%code requires {
    #pragma warning(disable:4127)

    #include <variant>
    #include "../ASM/TypeDefs.h"
    #include "../ASM/Expressions.h"
    #include "../ASM/Exceptions.h"
    namespace yy
    {
        class Lexer;
    }
}

%define api.token.raw
%define api.value.type{ std::variant<std::string, Value, UnOp::Jmp, UnOp::Shift> }

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
    #include "../Utils/Logger.h"

    #undef ERROR

    namespace details
    {
        std::vector<std::unique_ptr<Input>> input;
        std::queue<Expr> exprs;
        std::unordered_map<std::shared_ptr<Label>, int32_t, Label::PtrHash, Label::PtrEqual> labels;
        std::string lastLabel;
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
    CALL
    CLNZ
    JSP
    JSNZ
    JSNC4
    RET
    PUSH
    POP
    END_LDM
    LSL
    LSR
    CSL
    CSR
    CDSL
    CDSR
    ADSL
    ADSR
    LSLQ
    LSRQ
    CSLQ
    CSRQ
    CDSLQ
    CDSRQ
    ADSLQ
    ADSRQ
    SET
;

%destructor                                         {
                                                        for (const auto &[lbl, pos] : details::labels)
                                                        {
                                                            if (pos == -1)
                                                                throw yy::parser::syntax_error(@$, "����� '" + lbl->GetStr() + "' �� �������!");
                                                        }
                                                    } END

%start program

%%

program:    blocks
;

blocks:     block blocks
|           %empty
;

block:      expr
|           error
;

expr:       binexpr SEMICOLON                       { flushExprs(); }
|           unexpr  SEMICOLON                       { flushExprs(); }
|           LABEL   COLON                           {
                                                        auto lbl = std::make_shared<Label>(std::get<std::string>($1));
                                                        lbl->IncrAddr(Publisher::GetInstance()->Size());
                                                        if (details::labels.contains(lbl))
                                                        {
                                                            if (details::labels[lbl] != -1)
                                                                throw yy::parser::syntax_error(lexer.getlocation(), "����� '" + lbl->GetStr() +  "' ��� ����������!");
                                                            else
                                                            {
                                                                details::labels[lbl] = lexer.getlocation().begin.line;
                                                                auto node = details::labels.extract(lbl);
                                                                node.key()->SetAddr(lbl->GetAddr());
                                                                details::labels.insert(std::move(node));
                                                            }
                                                        }
                                                        else
                                                        {
                                                            auto [it, ok] = details::labels.emplace(std::move(lbl), lexer.getlocation().begin.line);
                                                            if (!ok)
                                                                throw InternalCompilerError("�� ������� �������� ����� � ���-�������");
                                                        }
                                                    }
;

binexpr:    ADD binexprf                            {
                                                        details::exprs.push(std::make_unique<BinOp>(BinOp::Op::ADD, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                                        details::input.pop_back();
                                                        LOG(INFO) << lexer.getlocation() << "\tMTEMU ADD:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                                    }
|           SUB binexprf                            {
                                                        details::exprs.push(std::make_unique<BinOp>(BinOp::Op::SUB, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                                        details::input.pop_back();
                                                        LOG(INFO) << lexer.getlocation() << "\tMTEMU SUB:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                                    }
|           MUL REG COMMA REG COMMA REG COMMA REG   {
                                                        Register r1(std::get<std::string>($2));
                                                        Register r2(std::get<std::string>($4));
                                                        Register r3(std::get<std::string>($6));
                                                        Register r4(std::get<std::string>($8));
                                                        if (r1 == r2 || r1 == r3 || r1 == r4 || r2 == r3 || r2 == r4 || r3 == r4)
                                                            throw yy::parser::syntax_error(lexer.getlocation(), "��� �������� � ������� ��������� ������ ���� ��������!");
                                                        if (r1.isRQ() || r2.isRQ() || r3.isRQ() || r4.isRQ())
                                                            throw yy::parser::syntax_error(lexer.getlocation(), "������������� �������� Q � ������� ��������� �� ��������������!");
                                                        BinCmd cmd(BinCmd::MulCmd, r1, r2, r3, r4);
                                                        details::exprs.swap(cmd.Get());
                                                    }
|           DIV binexprf                            {
                                                        throw yy::parser::syntax_error(lexer.getlocation(), "������� ������� �� ��������������!");
                                                    }
|           OR binexprf                             {
                                                        details::exprs.push(std::make_unique<BinOp>(BinOp::Op::OR, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                                        details::input.pop_back();
                                                        LOG(INFO) << lexer.getlocation() << "\tMTEMU OR:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                                    }
|           AND binexprf                            {
                                                        details::exprs.push(std::make_unique<BinOp>(BinOp::Op::AND, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                                        details::input.pop_back();
                                                        LOG(INFO) << lexer.getlocation() << "\tMTEMU AND:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                                    }
|           XOR binexprf                            {
                                                        details::exprs.emplace(std::make_unique<BinOp>(BinOp::Op::XOR, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                                        details::input.pop_back();
                                                        LOG(INFO) << lexer.getlocation() << "\tMTEMU XOR:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                                    }
|           NXOR binexprf                           {
                                                        details::exprs.push(std::make_unique<BinOp>(BinOp::Op::NXOR, *(dynamic_cast<BinOpIn *>(details::input.back().get()))));
                                                        details::input.pop_back();
                                                        LOG(INFO) << lexer.getlocation() << "\tMTEMU NXOR:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                                    }
;

binexprf:	REG COMMA REG COMMA REG                 {
                                                        Register r1(std::get<std::string>($1));
                                                        Register r2(std::get<std::string>($3));
                                                        Register r3(std::get<std::string>($5));
                                                        if (r1 != r2 && r2 != r3 && !r1.isRQ() && !r2.isRQ() && !r3.isRQ())
                                                            throw yy::parser::syntax_error(lexer.getlocation(), "������������� 3 ��������� ��������� ������ ���������� � �������� ��������� �� ��������������!");
                                                        if (r2.isRQ() && r3.isRQ())
                                                            throw yy::parser::syntax_error(lexer.getlocation(),"������� Q �� ����� ���� ������������ ����� � ������ ��������� �������� ��������!");
                                    
                                                        details::input.push_back(std::make_unique<BinOpIn>(r1, r2, r3));
                                                    }
|           REG COMMA REG COMMA NUM                 { details::input.push_back(std::make_unique<BinOpIn>(Register(std::get<std::string>($1)), Register(std::get<std::string>($3)), std::get<Value>($5))); }
|           REG COMMA NUM COMMA REG                 { details::input.push_back(std::make_unique<BinOpIn>(Register(std::get<std::string>($1)), std::get<Value>($3), Register(std::get<std::string>($5)))); }
|           REG COMMA REG                           {
                                                        Register r1(std::get<std::string>($1));
                                                        Register r2(std::get<std::string>($3));
                                                        if (r1.isRQ() && r2.isRQ())
                                                            throw yy::parser::syntax_error(lexer.getlocation(), "������� Q �� ����� ���� ������������ ����� � ������ ��������� �������� ��������!");
                                                        details::input.push_back(std::make_unique<BinOpIn>(r1, r2));
                                                    }
|           REG COMMA NUM                           { details::input.push_back(std::make_unique<BinOpIn>(Register(std::get<std::string>($1)), std::get<Value>($3))); }
|           NUM COMMA REG                           { details::input.push_back(std::make_unique<BinOpIn>(std::get<Value>($1), Register(std::get<std::string>($3)))); }
;

unexpr:     jumplbl LABEL                           {
                                                        auto lbl = std::make_shared<Label>(std::get<std::string>($2));
                                                        if (!details::labels.contains(lbl))
                                                        {
                                                            details::labels.emplace(lbl, -1);
                                                            details::exprs.push(std::make_unique<UnOp>(std::get<UnOp::Jmp>($1), lbl));
                                                        }
                                                        else
                                                        {
                                                            auto node = details::labels.extract(lbl);
                                                            details::exprs.push(std::make_unique<UnOp>(std::get<UnOp::Jmp>($1), node.key()));
                                                        }
                                                        LOG(INFO) << lexer.getlocation() << "\tMTEMU JUMP:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                                    }
|           jumpnolbl                               { details::exprs.push(std::make_unique<UnOp>(std::get<UnOp::Jmp>($1))); }
|           shift REG                               { 
                                                        Register r(std::get<std::string>($2));
                                                        if (r.isRQ())
                                                            throw yy::parser::syntax_error(lexer.getlocation(), "������� Q �� ����� ���� ��������� ������");
                                                        details::exprs.push(std::make_unique<UnOp>(std::get<UnOp::Shift>($1), r));
                                                        LOG(INFO) << lexer.getlocation() << "\tMTEMU SHIFT:\t" << details::exprs.front()->ToMtemuFmt() << std::endl;
                                                    }
|           SET REG COMMA REG                       { details::exprs.push(std::make_unique<UnOp>(UnOp::SetOp, Register(std::get<std::string>($2)), Register(std::get<std::string>($4)))); }
|           SET REG COMMA NUM                       { details::exprs.push(std::make_unique<UnOp>(UnOp::SetOp, Register(std::get<std::string>($2)), std::get<Value>($4))); }
;

jumplbl:    JNZ                                     { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JNZ); }
|           JMP                                     { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JMP); }
|           JZ                                      { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JZ); }
|           JF3                                     { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JF3); }
|           JOVR                                    { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JOVR); }
|           JC4                                     { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JC4); }
|           CALL                                    { $$.emplace<UnOp::Jmp>(UnOp::Jmp::CALL); }
|           CLNZ                                    { $$.emplace<UnOp::Jmp>(UnOp::Jmp::CLNZ); }
;

jumpnolbl:  JSP                                     { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JSP); }
|           JSNZ                                    { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JSNZ); }
|           JSNC4                                   { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JSNC4); }
|           RET                                     { $$.emplace<UnOp::Jmp>(UnOp::Jmp::RET); }
|           PUSH                                    { $$.emplace<UnOp::Jmp>(UnOp::Jmp::PUSH); }
|           POP                                     { $$.emplace<UnOp::Jmp>(UnOp::Jmp::POP); }
|           END_LDM                                 { $$.emplace<UnOp::Jmp>(UnOp::Jmp::END_LDM); }
;

shift:      LSL                                     { $$.emplace<UnOp::Shift>(UnOp::Shift::LSL); }
|           LSR                                     { $$.emplace<UnOp::Shift>(UnOp::Shift::LSR); }
|           CSL                                     { $$.emplace<UnOp::Shift>(UnOp::Shift::CSL); }
|           CSR                                     { $$.emplace<UnOp::Shift>(UnOp::Shift::CSR); }
|           CDSL                                    { $$.emplace<UnOp::Shift>(UnOp::Shift::CDSL); }
|           CDSR                                    { $$.emplace<UnOp::Shift>(UnOp::Shift::CDSR); }
|           ADSL                                    { $$.emplace<UnOp::Shift>(UnOp::Shift::ADSL); }
|           ADSR                                    { $$.emplace<UnOp::Shift>(UnOp::Shift::ADSR); }
|           LSLQ                                    { $$.emplace<UnOp::Shift>(UnOp::Shift::LSLQ); }
|           LSRQ                                    { $$.emplace<UnOp::Shift>(UnOp::Shift::LSRQ); }
|           CSLQ                                    { $$.emplace<UnOp::Shift>(UnOp::Shift::CSLQ); }
|           CSRQ                                    { $$.emplace<UnOp::Shift>(UnOp::Shift::CSRQ); }
|           CDSLQ                                   { $$.emplace<UnOp::Shift>(UnOp::Shift::CDSLQ); }
|           CDSRQ                                   { $$.emplace<UnOp::Shift>(UnOp::Shift::CDSRQ); }
|           ADSLQ                                   { $$.emplace<UnOp::Shift>(UnOp::Shift::ADSLQ); }
|           ADSRQ                                   { $$.emplace<UnOp::Shift>(UnOp::Shift::ADSRQ); }
;

%%

void yy::parser::error(const location_type &loc, const std::string &err_message)
{
    LOG(ERROR) << "���: " << loc.begin.line << " ����: " << loc.begin.column << ". ������: " << err_message << std::endl;
}

void flushExprs()
{
    if (details::exprs.empty())
        return;

    if (details::exprs.size() == 1)
        Publisher::GetInstance()->Push(std::move(details::exprs.front()));
    else
        Publisher::GetInstance()->Push(details::exprs);

    while (!details::exprs.empty())
    {    
        details::exprs.pop();
    }
}
