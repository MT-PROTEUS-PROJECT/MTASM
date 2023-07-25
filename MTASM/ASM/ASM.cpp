#include "ASM.h"

namespace yy
{
    ASM::ASM(std::istream &in, std::ostream &out) : _lexer(in), _parser(*this), _publisher(out) {}

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

    location ASM::GetLocation() const
    {
        return _lexer.getlocation();
    }
}
