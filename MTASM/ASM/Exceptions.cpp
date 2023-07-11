#include "Exceptions.h"

InternalCompilerError::InternalCompilerError(std::string message) : _message(std::move(message))
{}

const char *InternalCompilerError::what() const noexcept
{
    return _message.c_str();
}
