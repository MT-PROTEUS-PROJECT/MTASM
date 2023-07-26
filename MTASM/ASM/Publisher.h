#pragma once

#include <ostream>
#include <vector>
#include <memory>

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
    uint32_t _cexprs;
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
    void Push(std::vector<Expr> &qexpr);

    uint32_t Size() const noexcept;
};
