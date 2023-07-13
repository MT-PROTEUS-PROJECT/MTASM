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


class UnOp final : public Expression
{
public:
    enum Op
    {
        JNZ = 0,
        JMP = 1,
        JZ = 12,
        JF3 = 13,
        JOVR = 14,
        JC4 = 15
    };
public:
    explicit UnOp(UnOp::Op opTag, std::string lbl) noexcept;

    ~UnOp() = default;
};

