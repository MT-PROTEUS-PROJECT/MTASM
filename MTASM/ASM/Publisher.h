#pragma once

#include <ostream>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include "Expressions.h"
#include "TypeDefs.h"

namespace yy
{
    class ASM;
}

class Publisher final
{
private:
    friend yy::ASM;

    std::vector<Expr> _qexpr;
    Address::Value _cexprs;
    std::unordered_map<Address::Value, std::string> _addrTocmd;
    std::ostream _out;

private:
    Publisher(std::ostream &out);

    void Write();

public:
    Publisher(const Publisher &) = delete;
    Publisher &operator=(const Publisher &) = delete;

    Publisher(Publisher &&) = delete;
    Publisher &operator=(Publisher &&) = delete;

    ~Publisher();

public:
    void Push(Expr &&expr);
    Address::Value Push(std::vector<Expr> &qexpr, const std::string &cmd);
};
