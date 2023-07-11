#pragma once

#include <stdexcept>

class InternalCompilerError final : public std::exception
{
private:
    std::string _message;

public:
    explicit InternalCompilerError(std::string message);

    const char *what() const noexcept override;
};
