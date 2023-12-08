#pragma once

#include "Parser.tab.hh"
#include <Lexer/Lexer.h>
#include <ASM/Publisher.h>
#include <ASM/Exceptions.h>

#include <string>

namespace yy
{
    class ASM final
    {
    public:
        struct _details
        {
            std::unique_ptr<Input> input;
            std::vector<Expr> exprs;
            std::unordered_map<std::string, Address::Value> cmdId;
            std::shared_ptr<std::string> curCmd = std::make_shared<std::string>();
            std::unordered_map<std::shared_ptr<Label>, Address::Value, Label::PtrHash, Label::PtrEqual> labels;
        } details;

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
