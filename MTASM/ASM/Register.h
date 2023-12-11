#pragma once

#include "Address.h"
#include <string>


class Register final
{
private:
    std::string _reg;

public:
    struct Hash
    {
        size_t operator()(const Register &r) const;
    };

public:
    explicit Register(std::string reg);
    
    Register(const Register &) = default;
    Register &operator=(const Register &) = default;

    Register(Register &&) = default;
    Register &operator=(Register &&) = default;
    
    ~Register() = default;

public:
    Address addr() const;
    bool isRQ() const;
    bool isTemplate() const noexcept;
    const std::string &get() const;

    static Register Next(const Register &r);

    friend bool operator==(const Register &lhs, const Register &rhs);
};
