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
    void binOp(yy::ASM &mtasm, BinOp::Op tag);
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
    LEFT_BRACE
    RIGHT_BRACE
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
    MOV
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
|           ID LEFT_BRACE args RIGHT_BRACE LEFT_CURLY_BRACE blocks RIGHT_CURLY_BRACE
                                                            {
                                                                pushCmd(mtasm, std::get<std::string>($1));
                                                            }
;

args:       REG                                             {
                                                                Register r(std::get<std::string>($1));
                                                                if (!r.isTemplate())
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_INVALID_ARGS);
                                                                    break;
                                                                }
                                                                mtasm.details.curCmd->args.push_back(std::move(r)); 
                                                            }
|           REG COMMA REG                                   {
                                                                Register r1(std::get<std::string>($1));
                                                                Register r2(std::get<std::string>($3));
                                                                if (!r1.isTemplate() || !r2.isTemplate())
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_INVALID_ARGS);
                                                                    break;
                                                                }
                                                                if (r1 == r2)
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_SAME_ARGS);
                                                                    break;
                                                                }
                                                                mtasm.details.curCmd->args.push_back(std::move(r1));
                                                                mtasm.details.curCmd->args.push_back(std::move(r2));
                                                            }
|           REG COMMA REG COMMA REG                         {
                                                                Register r1(std::get<std::string>($1));
                                                                Register r2(std::get<std::string>($3));
                                                                Register r3(std::get<std::string>($5));
                                                                if (!r1.isTemplate() || !r2.isTemplate() || !r3.isTemplate())
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_INVALID_ARGS);
                                                                    break;
                                                                }
                                                                if (r1 == r2 || r1 == r3 || r2 == r3)
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_SAME_ARGS);
                                                                    break;
                                                                }
                                                                mtasm.details.curCmd->args.push_back(std::move(r1));
                                                                mtasm.details.curCmd->args.push_back(std::move(r2));
                                                                mtasm.details.curCmd->args.push_back(std::move(r3));
                                                            }
;

specargs:   REG                                             {
                                                                mtasm.details.specArgs.clear();
                                                                Register r(std::get<std::string>($1));
                                                                if (r.isTemplate() && !mtasm.details.curCmd->HasArg(r))
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_INVALID_INSTANCE);
                                                                    break;
                                                                }
                                                                mtasm.details.specArgs.push_back(std::move(r));
                                                            }
|           REG COMMA REG                                   {
                                                                mtasm.details.specArgs.clear();
                                                                Register r1(std::get<std::string>($1));
                                                                Register r2(std::get<std::string>($3));
                                                                if ((r1.isTemplate() && !mtasm.details.curCmd->HasArg(r1)) ||
                                                                    (r2.isTemplate() && !mtasm.details.curCmd->HasArg(r2)))
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_INVALID_INSTANCE);
                                                                    break;
                                                                }
                                                                mtasm.details.specArgs.push_back(std::move(r1));
                                                                mtasm.details.specArgs.push_back(std::move(r2));
                                                            }
|           REG COMMA REG COMMA REG                         {   
                                                                mtasm.details.specArgs.clear();
                                                                Register r1(std::get<std::string>($1));
                                                                Register r2(std::get<std::string>($3));
                                                                Register r3(std::get<std::string>($5));
                                                                if ((r1.isTemplate() && !mtasm.details.curCmd->HasArg(r1)) ||
                                                                    (r2.isTemplate() && !mtasm.details.curCmd->HasArg(r2)) ||
                                                                    (r3.isTemplate() && !mtasm.details.curCmd->HasArg(r3)))
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_INVALID_INSTANCE);
                                                                    break;
                                                                }
                                                                mtasm.details.specArgs.push_back(std::move(r1));
                                                                mtasm.details.specArgs.push_back(std::move(r2));
                                                                mtasm.details.specArgs.push_back(std::move(r3));
                                                            }
;

blocks:     block
|           blocks block
;

block:      expr
;

expr:       binexpr
|           unexpr
|           ID   COLON                                      {
                                                                auto lbl = std::make_shared<Label>(mtasm.details.curCmdId, std::get<std::string>($1));
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
                                                                binOp(mtasm, BinOp::Op::ADD);
                                                            }
|           ADDC binexprf                                   {
                                                                binOp(mtasm, BinOp::Op::ADDC);
                                                            }
|           SUB binexprf                                    {
                                                                binOp(mtasm, BinOp::Op::SUB);
                                                            }
|           SUBC binexprf                                   {
                                                                binOp(mtasm, BinOp::Op::SUBC);
                                                            }
|           OR binexprf                                     {
                                                                binOp(mtasm, BinOp::Op::OR);
                                                            }
|           AND binexprf                                    {
                                                                binOp(mtasm, BinOp::Op::AND);
                                                            }
|           XOR binexprf                                    {
                                                                binOp(mtasm, BinOp::Op::XOR);
                                                            }
|           NXOR binexprf                                   {
                                                                binOp(mtasm, BinOp::Op::NXOR);
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
                                                                if ((r1.isTemplate() && !mtasm.details.curCmd->HasArg(r1)) ||
                                                                    (r2.isTemplate() && !mtasm.details.curCmd->HasArg(r2)) ||
                                                                    (r3.isTemplate() && !mtasm.details.curCmd->HasArg(r3))
                                                                )
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_HAS_NOT_SPEC_ARGS, mtasm.details.curCmd->name);
                                                                    break;
                                                                }
                                    
                                                                mtasm.details.input = std::make_unique<BinOpIn>(r1, r2, r3);
                                                            }
|           REG COMMA REG COMMA NUM                         {
                                                                Register r1(std::get<std::string>($1));
                                                                Register r2(std::get<std::string>($3));
                                                                if ((r1.isTemplate() && !mtasm.details.curCmd->HasArg(r1)) ||
                                                                    (r2.isTemplate() && !mtasm.details.curCmd->HasArg(r2)))
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_HAS_NOT_SPEC_ARGS, mtasm.details.curCmd->name);
                                                                    break;
                                                                }
                                                                mtasm.details.input = std::make_unique<BinOpIn>(r1, r2, std::get<Value>($5)); 
                                                            }
|           REG COMMA NUM COMMA REG                         { 
                                                                Register r1(std::get<std::string>($1));
                                                                Register r2(std::get<std::string>($5));
                                                                if ((r1.isTemplate() && !mtasm.details.curCmd->HasArg(r1)) ||
                                                                    (r2.isTemplate() && !mtasm.details.curCmd->HasArg(r2)))
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_HAS_NOT_SPEC_ARGS, mtasm.details.curCmd->name);
                                                                    break;
                                                                }
                                                                mtasm.details.input = std::make_unique<BinOpIn>(r1, std::get<Value>($3), r2);
                                                            }
|           REG COMMA REG                                   {
                                                                Register r1(std::get<std::string>($1));
                                                                Register r2(std::get<std::string>($3));
                                                                if (r1.isRQ() && r2.isRQ())
                                                                {
                                                                    syntaxError(mtasm, SE::BIN_Q_L_R);
                                                                    break;
                                                                }
                                                                if ((r1.isTemplate() && !mtasm.details.curCmd->HasArg(r1)) ||
                                                                    (r2.isTemplate() && !mtasm.details.curCmd->HasArg(r2)))
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_HAS_NOT_SPEC_ARGS, mtasm.details.curCmd->name);
                                                                    break;
                                                                }
                                                                mtasm.details.input = std::make_unique<BinOpIn>(r1, r2);
                                                            }
|           REG COMMA NUM                                   { 
                                                                Register r(std::get<std::string>($1));
                                                                if ((r.isTemplate() && !mtasm.details.curCmd->HasArg(r)))
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_HAS_NOT_SPEC_ARGS, mtasm.details.curCmd->name);
                                                                    break;
                                                                }
                                                                mtasm.details.input = std::make_unique<BinOpIn>(r, std::get<Value>($3)); 
                                                            }
|           NUM COMMA REG                                   { 
                                                                Register r(std::get<std::string>($3));
                                                                if ((r.isTemplate() && !mtasm.details.curCmd->HasArg(r)))
                                                                {
                                                                    syntaxError(mtasm, SE::CMD_HAS_NOT_SPEC_ARGS, mtasm.details.curCmd->name);
                                                                    break;
                                                                }
                                                                mtasm.details.input = std::make_unique<BinOpIn>(std::get<Value>($1), r);
                                                            }
;

unexpr:     jumplbl ID                                      {
                                                                auto cmd_id = std::get<std::string>($2);
                                                                if (mtasm.details.cmdId.contains(cmd_id))
                                                                {
                                                                    if (mtasm.details.cmdId[cmd_id]->addr != Address::INVALID)
                                                                    {
                                                                        mtasm.details.exprs.push_back(std::make_shared<UnOp>(std::get<UnOp::Jmp>($1), std::make_shared<Label>(cmd_id, mtasm.details.cmdId[cmd_id]->addr), true));
                                                                        break;
                                                                    }
                                                                }
                                                    
                                                                auto lbl = std::make_shared<Label>(mtasm.details.curCmdId, std::get<std::string>($2));
                                                                if (!mtasm.details.labels.contains(lbl))
                                                                {
                                                                    mtasm.details.labels.emplace(lbl, Address::INVALID);
                                                                    mtasm.details.exprs.push_back(std::make_shared<UnOp>(std::get<UnOp::Jmp>($1), lbl));
                                                                }
                                                                else
                                                                {
                                                                    auto node = mtasm.details.labels.extract(lbl);
                                                                    mtasm.details.exprs.push_back(std::make_shared<UnOp>(std::get<UnOp::Jmp>($1), node.key()));
                                                                    mtasm.details.labels.insert(std::move(node));
                                                                }
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tJUMP\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           jumplbl ID LEFT_BRACE specargs RIGHT_BRACE      {
                                                                auto cmd_id = std::get<std::string>($2);
                                                                if (mtasm.details.cmdId.contains(cmd_id))
                                                                {
                                                                    if (mtasm.details.cmdId[cmd_id]->args.size() != mtasm.details.specArgs.size())
                                                                    {
                                                                        syntaxError(mtasm, SE::CMD_INVALID_ARGS_COUNT);
                                                                        break;
                                                                    }
                                                                        
                                                                    std::string cmd_id_with_args = cmd_id + "(";
                                                                    for (const auto &arg : mtasm.details.specArgs)
                                                                        cmd_id_with_args += arg.get() + ",";
                                                                    cmd_id_with_args.pop_back();
                                                                    cmd_id_with_args += ")";

                                                                    if (!mtasm.details.cmdId.count(cmd_id_with_args))
                                                                    {
                                                                        auto specCmd = std::make_shared<yy::ASM::cmd>(*mtasm.details.cmdId[cmd_id].get());
                                                                        specCmd->specArgs = mtasm.details.specArgs;
                                                                        specCmd->SpecTemplates();
                                                                        specCmd->name = cmd_id_with_args;
                                                                        specCmd->addr = mtasm.GetPublisher().Push(specCmd->exprs, specCmd->name);

                                                                        mtasm.details.cmdId[cmd_id_with_args] = specCmd;
                                                                    }
                                                                    
                                                                    mtasm.details.exprs.push_back(std::make_shared<UnOp>(std::get<UnOp::Jmp>($1), std::make_shared<Label>(cmd_id_with_args, mtasm.details.cmdId[cmd_id_with_args]->addr), true));
                                                                }
                                                            }
|           jumpnolbl                                       { mtasm.details.exprs.push_back(std::make_shared<UnOp>(std::get<UnOp::Jmp>($1))); }
|           shift REG                                       { 
                                                                Register r(std::get<std::string>($2));
                                                                if (r.isRQ())
                                                                {
                                                                    syntaxError(mtasm, SE::SHIFT_Q);
                                                                    break;
                                                                }
                                                                mtasm.details.exprs.push_back(std::make_shared<UnOp>(std::get<UnOp::Shift>($1), r));
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tSHIFT\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           MOV REG COMMA REG                               { 
                                                                mtasm.details.exprs.push_back(std::make_shared<UnOp>(UnOp::SetOp, Register(std::get<std::string>($2)), Register(std::get<std::string>($4))));
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tSET\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           MOV REG COMMA NUM                               { 
                                                                mtasm.details.exprs.push_back(std::make_shared<UnOp>(UnOp::SetOp, Register(std::get<std::string>($2)), std::get<Value>($4)));
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tSET\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
                                                            }
|           GET REG                                         {
                                                                mtasm.details.exprs.push_back(std::make_shared<UnOp>(UnOp::GetOp, Register(std::get<std::string>($2))));
                                                                LOG(DEBUG) << mtasm.GetLocation() << "\tGET\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
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
    for (const auto &expr : mtasm.details.exprs)
    {
        if (!expr->CheckTemplates(mtasm.details.curCmd->args))
        {
            syntaxError(mtasm, SE::CMD_HAS_NOT_SPEC_ARGS, mtasm.details.curCmd->name);
            return false;
        }
    }
    mtasm.details.curCmd->name = id;
    mtasm.details.curCmd->addr = Address::INVALID;
    mtasm.details.curCmd->exprs = mtasm.details.exprs;

    if (auto [it, ok] = mtasm.details.cmdId.emplace(id, mtasm.details.curCmd); !ok)
        throw InternalCompilerError("Не удалось добавить название команды в хэш-таблицу");

    *(mtasm.details.curCmdId.get()) = id;
    mtasm.details.curCmdId = std::make_shared<std::string>();
    mtasm.details.curCmd = std::make_shared<yy::ASM::cmd>();

    return true;
}

void pushCmd(yy::ASM &mtasm, const std::string &id)
{
    if (!insertCmdId(mtasm, id))
        return;
    if (mtasm.details.cmdId[id]->args.empty())
        mtasm.details.cmdId[id]->addr = flushExprs(mtasm, id);
    mtasm.details.exprs.clear();
}

Address::Value flushExprs(yy::ASM &mtasm, const std::string &cmd)
{
    Address::Value addr = Address::INVALID;
    if (mtasm.details.exprs.empty())
        return addr;

    addr = mtasm.GetPublisher().Push(mtasm.details.exprs, cmd);

    return addr;
}

void binOp(yy::ASM &mtasm, BinOp::Op tag)
{
    if (!mtasm.details.input)
        return;
    mtasm.details.exprs.push_back(std::make_shared<BinOp>(tag, *(dynamic_cast<BinOpIn *>(mtasm.details.input.get()))));
    mtasm.details.input.reset();
    LOG(DEBUG) << mtasm.GetLocation() << "\t" << [tag]()
    {
        switch (tag)
        {
        case BinOp::Op::ADD:
            return "ADD";
        case BinOp::Op::ADDC:
            return "ADDC";
        case BinOp::Op::SUB:
            return "SUB";
        case BinOp::Op::SUBC:
            return "SUBC";
        case BinOp::Op::OR:
            return "OR";
        case BinOp::Op::AND:
            return "AND";
        case BinOp::Op::XOR:
            return "XOR";
        case BinOp::Op::NXOR:
            return "NXOR";
        default:
            return "UNKNOWN";
        }
    }() << "\t" << mtasm.details.exprs.back()->ToMtemuFmt() << std::endl;
}
