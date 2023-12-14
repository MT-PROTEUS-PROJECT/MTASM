#include "ASM.h"

#include <algorithm>

namespace yy
{
    ASM::cmd::cmd(const cmd& rhs) : name(rhs.name), addr(rhs.addr), args(rhs.args), specArgs(rhs.specArgs)
    {
        for (const auto& expr : rhs.exprs)
        {
            if (auto binop = dynamic_cast<BinOp*>(expr.get()); binop != nullptr)
            {
                exprs.push_back(std::make_shared<BinOp>(*binop));
            }
            else
            {
                exprs.push_back(std::make_shared<UnOp>(*dynamic_cast<UnOp*>(expr.get())));
            }
        }
    }

    ASM::cmd& ASM::cmd::operator=(const ASM::cmd& rhs)
    {
        if (this == &rhs)
            return *this;

        ASM::cmd new_cmd(rhs);
        *this = std::move(new_cmd);

        return *this;
    }

    void ASM::cmd::SpecTemplates()
    {
        for (auto& expr : exprs)
        {
            expr->SpecTemplates(args, specArgs);
        }
    }

    bool ASM::cmd::HasArg(const Register& r)
    {
        return std::find(args.begin(), args.end(), r) != args.end();
    }

    ASM::ASM(std::istream &in, std::ostream &out) : _lexer(in), _parser(*this), _publisher(out)
    {
        translator["COMMA"] = ",";
        translator["SEMICOLON"] = ";";
        translator["COLON"] = ":";
        translator["LEFT_CURLY_BRACE"] = "{";
        translator["RIGHT_CURLY_BRACE"] = "}";
        translator["LEFT_BRACE"] = "(";
        translator["RIGHT_BRACE"] = ")";
        translator["NUM"] = "◊»—ÀŒ";
        translator["ID"] = "»ƒ≈Õ“»‘» ¿“Œ–";
        translator["REG"] = "POH";
        translator["END"] = " ŒÕ≈÷ œ–Œ√–¿ÃÃ€";
    }

    yy::Lexer &ASM::GetLexer() noexcept
    {
        return _lexer;
    }

    ::Publisher &ASM::GetPublisher() noexcept
    {
        return _publisher;
    }

    ::ExceptionContainer &ASM::GetEC() noexcept
    {
        return _exceptions;
    }

    int ASM::Parse()
    {
        try
        {
            return _parser();
        }
        catch (const InternalCompilerError &ex)
        {
            _exceptions.Push(ExceptionContainer::Tag::ICE, std::nullopt, ex.what());
        }
        catch (const std::exception &ex)
        {
            _exceptions.Push(ExceptionContainer::Tag::OTHER, std::nullopt, ex.what());
        }
        return -1;
    }

    std::vector<std::string> ASM::GetCmds()
    {
        std::vector<std::string> cmds(details.cmdId.size());
        size_t i = 0;
        for (const auto& [key, value] : details.cmdId)
        {
            if (!value->args.empty() && value->specArgs.empty())
            {
                auto specKey = key + "(";
                for (const auto& arg : value->args)
                    specKey += arg.get() + ",";
                specKey.pop_back();
                specKey += ")";
                cmds[i++] = specKey;
            }
            else
            {
                cmds[i++] = key;
            }
        }

        return cmds;
    }

    location ASM::GetLocation() const
    {
        return _lexer.getlocation();
    }
}
