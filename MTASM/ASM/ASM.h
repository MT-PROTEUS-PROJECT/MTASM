#pragma once

#include "Parser.tab.hh"
#include <Lexer/Lexer.h>
#include <ASM/Publisher.h>
#include <ASM/Exceptions.h>

#include <string>
#include <vector>

namespace yy
{
    class ASM final
    {
    public:
        struct cmd
        {
            std::string name;
            Address::Value addr;
            std::vector<Register> args;
            std::vector<Register> specArgs;
            std::vector<Expr> exprs;

            cmd() = default;

            cmd(const cmd& rhs);
            cmd& operator=(const cmd& rhs);

            cmd(cmd&&) noexcept = default;
            cmd& operator=(cmd&&) noexcept = default;

            void SpecTemplates();
            bool HasArg(const Register& r);
        };

        struct _details
        {
            std::unique_ptr<Input> input;
            std::vector<Expr> exprs;
            std::unordered_map<std::string, std::shared_ptr<cmd>> cmdId;
            std::shared_ptr<std::string> curCmdId = std::make_shared<std::string>();
            std::shared_ptr<cmd> curCmd = std::make_shared<cmd>();
            std::vector<Register> specArgs;
            std::string curSpecCmd;
            std::unordered_map<std::shared_ptr<Label>, Address::Value, Label::PtrHash, Label::PtrEqual> labels;
        } details;
        std::unordered_map<std::string, std::string> translator;
    private:
        Lexer _lexer;
        parser _parser;
        ::Publisher _publisher;
        ::ExceptionContainer _exceptions;

    public:
        ASM(std::istream &in, std::ostream &out);

        Lexer &GetLexer() noexcept;
        ::Publisher &GetPublisher() noexcept;
        ::ExceptionContainer &GetEC() noexcept;

        std::vector<std::string> GetCmds();

        int Parse();
        location GetLocation() const;

        ~ASM() = default;
    };
}
