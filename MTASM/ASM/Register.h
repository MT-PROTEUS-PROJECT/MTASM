#pragma once

#include "Address.h"
#include <string>


class Register final
{
    std::string _reg;
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

    friend bool operator==(const Register &lhs, const Register &rhs);
};
