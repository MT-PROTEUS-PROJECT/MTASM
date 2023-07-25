#include "Exceptions.h"

InternalCompilerError::InternalCompilerError(std::string message) : std::runtime_error(_message), _message(std::move(message))
{}

const char *InternalCompilerError::what() const noexcept
{
    return _message.c_str();
}


void ExceptionContainer::Push(ExceptionContainer::Tag tag, std::string msg)
{
    _exceptions.emplace_back(tag, std::move(msg));
}

std::vector<ExceptionContainer::Exception> ExceptionContainer::Get(ExceptionContainer::Tag tag)
{
    std::vector<ExceptionContainer::Exception> res;
    for (const auto &exception : _exceptions)
    {
        if (exception._tag == tag)
            res.push_back(exception);
    }

    return res;
}
