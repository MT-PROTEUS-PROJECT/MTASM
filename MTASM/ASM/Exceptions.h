#pragma once

#include <stdexcept>
#include <vector>

class InternalCompilerError final : public std::runtime_error
{
private:
    std::string _message;

public:
    explicit InternalCompilerError(std::string message);

    const char *what() const noexcept override;
};

namespace yy
{
    class ASM;
}

class ExceptionContainer final
{
public:
    enum class Tag
    {
        ICE, // INTERNAL COMPILER ERROR
        SE, // SYNTAX ERROR
        OTHER
    };

    struct Exception
    {
        ExceptionContainer::Tag _tag;
        std::string _msg;
    };

private:
    friend yy::ASM;

    std::vector<Exception> _exceptions;
    
    ExceptionContainer() = default;

public:
    ExceptionContainer(const ExceptionContainer &) = delete;
    ExceptionContainer &operator=(const ExceptionContainer &) = delete;

    ExceptionContainer(ExceptionContainer &&) = delete;
    ExceptionContainer &operator=(ExceptionContainer &&) = delete;

    ~ExceptionContainer() = default;

public:
    void Push(ExceptionContainer::Tag tag, std::string msg);
    std::vector<Exception> Get(ExceptionContainer::Tag tag);
};
