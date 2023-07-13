#pragma once

#include "Input.h"

class Expression
{
protected:
    uint32_t _mtemuFmt = 2; // JNXT
public:
    Expression() = default;

    uint32_t ToMtemuFmt() const noexcept;

    virtual ~Expression() = default;
};

class BinOp final : public Expression
{
public:
    enum Op
    {
    // Arithmetic
        ADD = 0,
        SUB = 9,
        MUL,
        DIV,
    // Logical
        OR = 3,
        AND = 4,
        XOR = 6,
        NXOR = 7
    };

private:
    void CommutativeOp(BinOp::Op opTag, const BinOpIn &in);

    void SubOp(const BinOpIn &in);
public:
    BinOp(BinOp::Op opTag, const BinOpIn &in);

    ~BinOp() = default;
};
