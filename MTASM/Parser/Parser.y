%language "c++"
%require "3.2"
%defines
%locations

%code requires {
    #pragma warning(disable:4127)

    #include <variant>
    #include <vector>
    #include <memory>
    #include <unordered_map>
    #include <string>
    #include <sstream>
    #include <format>
    #include <string_view>

    #include <ASM/TypeDefs.h>
    #include <ASM/Expressions.h>

    namespace yy
    {
        class ASM;
    }
}

%define api.token.raw
%define api.value.type{ std::variant<std::string, Value, UnOp::Jmp, UnOp::Shift> }

%parse-param{ ASM &mtasm }
%code{
    #include <ASM/ASM.h>
    #include <ASM/Exceptions.h>
    #include <ASM/Input.h>
    #include <ASM/Publisher.h>
    #include <Utils/Logger.h>
    #include <ASM/Register.h>
    #include <ASM/Label.h>

    #ifdef yylex
        #undef	yylex
    #endif
    #define	yylex mtasm.GetLexer().yylex
    
    #undef ERROR
    
    Address::Value flushExprs(yy::ASM &mtasm, const std::string &cmd);
    template <typename... Args>
    void syntaxError(yy::ASM &mtasm, std::string_view fmt, Args&&... args);
    bool insertCmdId(yy::ASM &mtasm, const std::string &id);
    void pushCmd(yy::ASM &mtasm, const std::string &id);
}

%define parse.error verbose
%define parse.lac full

%define api.token.prefix{TOKEN_}
%token
    END 0
    COMMA
    SEMICOLON
    COLON
    LEFT_CURLY_BRACE
    RIGHT_CURLY_BRACE
    MAIN
    NUM
    REG
    ID
    ADD
    ADDC
    SUB
    SUBC
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
    GET
;

%destructor                                                 {
                                                                for (const auto &[lbl, pos] : mtasm.details.labels)
                                                                {
                                                                    if (pos == Address::INVALID)
                                                                        syntaxError(mtasm, SE::LBL_NOT_FOUND, lbl->GetStr());
                                                                }
                                                            } END

%start program

%%

program:    cmds main
|           %empty
;

main:       MAIN LEFT_CURLY_BRACE blocks RIGHT_CURLY_BRACE  {
                                                                pushCmd(mtasm, "MAIN");
                                                            }

;

cmds:       cmd cmds
|           %empty
;

cmd:        ID LEFT_CURLY_BRACE blocks RIGHT_CURLY_BRACE    {
                                                                pushCmd(mtasm, std::get<std::string>($1));
                                                            }
;

blocks:     block
|           blocks block
;

block:      expr
;

expr:       binexpr SEMICOLON
|           unexpr  SEMICOLON
|           ID   COLON                                      {
                                                                auto lbl = std::make_shared<Label>(mtasm.details.curCmd, std::get<std::string>($1));
                                                                lbl->IncrAddr(static_cast<Address::Value>(mtasm.details.exprs.size()));
                                                                if (mtasm.details.labels.contains(lbl))
                                                                {
                                                                    if (mtasm.details.labels[lbl] != Address::INVALID)
                                                                    {
                                                                        syntaxError(mtasm, SE::LBL_EXISTS, lbl->GetStr());
                                                                        break;
                                                                    }
                                                                    else
                                                                    {
                                                                        mtasm.details.labels[lbl] = mtasm.GetLocation().begin.line;
                                                                        auto node = mtasm.details.labels.extract(lbl);
                                                                        node.key()->SetAddr(lbl->GetAddr());
                                                                        mtasm.details.labels.insert(std::move(node));
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    if (auto [it, ok] = mtasm.details.labels.emplace(std::move(lbl), mtasm.GetLocation().begin.line); !ok)
                                                                        throw InternalCompilerError("Не удалось добавить метку в хэш-таблицу");
                                                                }
                                                            } 
;

binexpr:    ADD binexprf                                    {
                                                                if (!mtasm.details.input)
                                                                    break;
                                                                mtasm.details.exprs.push_back(std::make_unique<BinOp>(BinOp::Op::ADD, *(dynamic_cast<BinOpIn *>(mtasm.details.input.get()))));
                                                                mtasm.details.input.reset();
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM ADD:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           ADDC binexprf                                   {
                                                                if (!mtasm.details.input)
                                                                    break;
                                                                mtasm.details.exprs.push_back(std::make_unique<BinOp>(BinOp::Op::ADDC, *(dynamic_cast<BinOpIn *>(mtasm.details.input.get()))));
                                                                mtasm.details.input.reset();
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM ADDC:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           SUB binexprf                                    {
                                                                if (!mtasm.details.input)
                                                                    break;
                                                                mtasm.details.exprs.push_back(std::make_unique<BinOp>(BinOp::Op::SUB, *(dynamic_cast<BinOpIn *>(mtasm.details.input.get()))));
                                                                mtasm.details.input.reset();
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM SUB:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           SUBC binexprf                                   {
                                                                if (!mtasm.details.input)
                                                                    break;
                                                                mtasm.details.exprs.push_back(std::make_unique<BinOp>(BinOp::Op::SUBC, *(dynamic_cast<BinOpIn *>(mtasm.details.input.get()))));
                                                                mtasm.details.input.reset();
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM SUBC:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           MUL REG COMMA REG COMMA REG COMMA REG           {
                                                                Register r1(std::get<std::string>($2));
                                                                Register r2(std::get<std::string>($4));
                                                                Register r3(std::get<std::string>($6));
                                                                Register r4(std::get<std::string>($8));
                                                                if (r1 == r2 || r1 == r3 || r1 == r4 || r2 == r3 || r2 == r4 || r3 == r4)
                                                                {
                                                                    syntaxError(mtasm, SE::MUL_DIFF_REG);
                                                                    break;
                                                                }
                                                                if (r1.isRQ() || r2.isRQ() || r3.isRQ() || r4.isRQ())
                                                                {
                                                                    syntaxError(mtasm, SE::MUL_Q_REG);
                                                                    break;
                                                                }
                                                                BinCmd cmd(BinCmd::MulCmd, r1, r2, r3, r4);
                                                                mtasm.details.exprs.insert(mtasm.details.exprs.end(), std::make_move_iterator(cmd.Get().begin()), std::make_move_iterator(cmd.Get().end()));
                                                            }
|           DIV binexprf                                    {
                                                                syntaxError(mtasm, "Команда деления не поддерживается!");
                                                            }
|           OR binexprf                                     {
                                                                if (!mtasm.details.input)
                                                                    break;
                                                                mtasm.details.exprs.push_back(std::make_unique<BinOp>(BinOp::Op::OR, *(dynamic_cast<BinOpIn *>(mtasm.details.input.get()))));
                                                                mtasm.details.input.reset();
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM OR:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           AND binexprf                                    {
                                                                if (!mtasm.details.input)
                                                                    break;
                                                                mtasm.details.exprs.push_back(std::make_unique<BinOp>(BinOp::Op::AND, *(dynamic_cast<BinOpIn *>(mtasm.details.input.get()))));
                                                                mtasm.details.input.reset();
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM AND:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           XOR binexprf                                    {
                                                                if (!mtasm.details.input)
                                                                    break;
                                                                mtasm.details.exprs.push_back(std::make_unique<BinOp>(BinOp::Op::XOR, *(dynamic_cast<BinOpIn *>(mtasm.details.input.get()))));
                                                                mtasm.details.input.reset();
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM XOR:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           NXOR binexprf                                   {
                                                                if (!mtasm.details.input)
                                                                    break;
                                                                mtasm.details.exprs.push_back(std::make_unique<BinOp>(BinOp::Op::NXOR, *(dynamic_cast<BinOpIn *>(mtasm.details.input.get()))));
                                                                mtasm.details.input.reset();
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM NXOR:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
;

binexprf:	REG COMMA REG COMMA REG                         {
                                                                Register r1(std::get<std::string>($1));
                                                                Register r2(std::get<std::string>($3));
                                                                Register r3(std::get<std::string>($5));
                                                                if (r1 != r2 && r1 != r3 && r2 != r3 && !r1.isRQ() && !r2.isRQ() && !r3.isRQ())
                                                                {
                                                                    syntaxError(mtasm, SE::BIN_3_DIFF_REG);
                                                                    break;
                                                                }
                                                                if (r1 != r2 && r1 != r3 && r2 == r3)
                                                                {
                                                                    syntaxError(mtasm, SE::BIN_1_L_R);
                                                                    break;
                                                                }
                                                                if (r2.isRQ() && r3.isRQ())
                                                                {
                                                                    syntaxError(mtasm, SE::BIN_Q_L_R);
                                                                    break;
                                                                }
                                    
                                                                mtasm.details.input = std::make_unique<BinOpIn>(r1, r2, r3);
                                                            }
|           REG COMMA REG COMMA NUM                         { mtasm.details.input = std::make_unique<BinOpIn>(Register(std::get<std::string>($1)), Register(std::get<std::string>($3)), std::get<Value>($5)); }
|           REG COMMA NUM COMMA REG                         { mtasm.details.input = std::make_unique<BinOpIn>(Register(std::get<std::string>($1)), std::get<Value>($3), Register(std::get<std::string>($5))); }
|           REG COMMA REG                                   {
                                                                Register r1(std::get<std::string>($1));
                                                                Register r2(std::get<std::string>($3));
                                                                if (r1.isRQ() && r2.isRQ())
                                                                {
                                                                    syntaxError(mtasm, SE::BIN_Q_L_R);
                                                                    break;
                                                                }
                                                                mtasm.details.input = std::make_unique<BinOpIn>(r1, r2);
                                                            }
|           REG COMMA NUM                                   { mtasm.details.input = std::make_unique<BinOpIn>(Register(std::get<std::string>($1)), std::get<Value>($3)); }
|           NUM COMMA REG                                   { mtasm.details.input = std::make_unique<BinOpIn>(std::get<Value>($1), Register(std::get<std::string>($3))); }
;

unexpr:     jumplbl ID                                      {
                                                                auto lbl = std::make_shared<Label>(mtasm.details.curCmd, std::get<std::string>($2));
                                                                if (!mtasm.details.labels.contains(lbl))
                                                                {
                                                                    mtasm.details.labels.emplace(lbl, Address::INVALID);
                                                                    mtasm.details.exprs.push_back(std::make_unique<UnOp>(std::get<UnOp::Jmp>($1), lbl));
                                                                }
                                                                else
                                                                {
                                                                    auto node = mtasm.details.labels.extract(lbl);
                                                                    mtasm.details.exprs.push_back(std::make_unique<UnOp>(std::get<UnOp::Jmp>($1), node.key()));
                                                                    mtasm.details.labels.insert(std::move(node));
                                                                }
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM JUMP:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           jumpnolbl                                       { mtasm.details.exprs.push_back(std::make_unique<UnOp>(std::get<UnOp::Jmp>($1))); }
|           shift REG                                       { 
                                                                Register r(std::get<std::string>($2));
                                                                if (r.isRQ())
                                                                {
                                                                    syntaxError(mtasm, SE::SHIFT_Q);
                                                                    break;
                                                                }
                                                                mtasm.details.exprs.push_back(std::make_unique<UnOp>(std::get<UnOp::Shift>($1), r));
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM SHIFT:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           SET REG COMMA REG                               { 
                                                                mtasm.details.exprs.push_back(std::make_unique<UnOp>(UnOp::SetOp, Register(std::get<std::string>($2)), Register(std::get<std::string>($4))));
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM SET:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           SET REG COMMA NUM                               { 
                                                                mtasm.details.exprs.push_back(std::make_unique<UnOp>(UnOp::SetOp, Register(std::get<std::string>($2)), std::get<Value>($4)));
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM SET:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           GET REG                                         {
                                                                mtasm.details.exprs.push_back(std::make_unique<UnOp>(UnOp::GetOp, Register(std::get<std::string>($2))));
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM GET:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           ID                                              {
                                                                auto cmd_id = std::get<std::string>($1);
                                                                if (!mtasm.details.cmdId.contains(cmd_id))
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_NOT_FOUND, cmd_id);
                                                                    break;
                                                                }
                                                                if (mtasm.details.cmdId[cmd_id] == Address::INVALID)
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_INVALID_ADDR, cmd_id);
                                                                    break;
                                                                }
                                                                mtasm.details.exprs.push_back(std::make_unique<UnOp>(UnOp::Jmp::CALL, std::make_shared<Label>(cmd_id, mtasm.details.cmdId[cmd_id])));
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tMTASM CMD CALL:\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
;

jumplbl:    JNZ                                             { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JNZ); }
|           JMP                                             { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JMP); }
|           JZ                                              { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JZ); }
|           JF3                                             { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JF3); }
|           JOVR                                            { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JOVR); }
|           JC4                                             { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JC4); }
|           CALL                                            { $$.emplace<UnOp::Jmp>(UnOp::Jmp::CALL); }
|           CLNZ                                            { $$.emplace<UnOp::Jmp>(UnOp::Jmp::CLNZ); }
;

jumpnolbl:  JSP                                             { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JSP); }
|           JSNZ                                            { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JSNZ); }
|           JSNC4                                           { $$.emplace<UnOp::Jmp>(UnOp::Jmp::JSNC4); }
|           RET                                             { $$.emplace<UnOp::Jmp>(UnOp::Jmp::RET); }
|           PUSH                                            { $$.emplace<UnOp::Jmp>(UnOp::Jmp::PUSH); }
|           POP                                             { $$.emplace<UnOp::Jmp>(UnOp::Jmp::POP); }
|           END_LDM                                         { $$.emplace<UnOp::Jmp>(UnOp::Jmp::END_LDM); }
;

shift:      LSL                                             { $$.emplace<UnOp::Shift>(UnOp::Shift::LSL); }
|           LSR                                             { $$.emplace<UnOp::Shift>(UnOp::Shift::LSR); }
|           CSL                                             { $$.emplace<UnOp::Shift>(UnOp::Shift::CSL); }
|           CSR                                             { $$.emplace<UnOp::Shift>(UnOp::Shift::CSR); }
|           CDSL                                            { $$.emplace<UnOp::Shift>(UnOp::Shift::CDSL); }
|           CDSR                                            { $$.emplace<UnOp::Shift>(UnOp::Shift::CDSR); }
|           ADSL                                            { $$.emplace<UnOp::Shift>(UnOp::Shift::ADSL); }
|           ADSR                                            { $$.emplace<UnOp::Shift>(UnOp::Shift::ADSR); }
|           LSLQ                                            { $$.emplace<UnOp::Shift>(UnOp::Shift::LSLQ); }
|           LSRQ                                            { $$.emplace<UnOp::Shift>(UnOp::Shift::LSRQ); }
|           CSLQ                                            { $$.emplace<UnOp::Shift>(UnOp::Shift::CSLQ); }
|           CSRQ                                            { $$.emplace<UnOp::Shift>(UnOp::Shift::CSRQ); }
|           CDSLQ                                           { $$.emplace<UnOp::Shift>(UnOp::Shift::CDSLQ); }
|           CDSRQ                                           { $$.emplace<UnOp::Shift>(UnOp::Shift::CDSRQ); }
|           ADSLQ                                           { $$.emplace<UnOp::Shift>(UnOp::Shift::ADSLQ); }
|           ADSRQ                                           { $$.emplace<UnOp::Shift>(UnOp::Shift::ADSRQ); }
;

%%

template <typename... Args>
void syntaxError(yy::ASM &mtasm, std::string_view fmt, Args&&... args)
{
    mtasm.GetEC().Push(ExceptionContainer::Tag::SE, mtasm.GetLocation(), std::vformat(fmt, std::make_format_args(args...)));
}

void yy::parser::error(const location_type &, const std::string &err_message)
{
    mtasm.GetEC().Push(ExceptionContainer::Tag::SE, mtasm.GetLocation(), err_message);
}

bool insertCmdId(yy::ASM &mtasm, const std::string &id)
{
    if (mtasm.details.cmdId.contains(id))
    {
        syntaxError(mtasm, SE::CMD_EXISTS, id);
        return false;
    }
    if (auto [it, ok] = mtasm.details.cmdId.emplace(id, Address::INVALID); !ok)
        throw InternalCompilerError("Не удалось добавить название команды в хэш-таблицу");

    *(mtasm.details.curCmd.get()) = id;
    mtasm.details.curCmd = std::make_shared<std::string>();

    return true;
}

void pushCmd(yy::ASM &mtasm, const std::string &id)
{
    if (!insertCmdId(mtasm, id))
        return;
    mtasm.details.cmdId[id] = flushExprs(mtasm, id);
}

Address::Value flushExprs(yy::ASM &mtasm, const std::string &cmd)
{
    Address::Value addr = Address::INVALID;
    if (mtasm.details.exprs.empty())
        return addr;

    addr = mtasm.GetPublisher().Push(mtasm.details.exprs, cmd);
    mtasm.details.exprs.clear();

    return addr;
}
