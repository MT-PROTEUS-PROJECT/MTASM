#pragma once

#include "Input.h"

class Expression
{
public:
    Expression() = default;

    virtual uint32_t ToMtemuFmt() const = 0;

    virtual ~Expression() = default;
};

class ArOp final : public Expression
{
private:
    uint32_t _mtemuFmt = 2; // JNXT

    void AddOp(const ArOpIn &in);

    void SubOp(const ArOpIn &in);

public:
    enum class Op
    {
        ADD,
        SUB,
        MUL,
        DIV
    };

public:
    ArOp(ArOp::Op opTag, const ArOpIn &in);

    uint32_t ToMtemuFmt() const override;

    ~ArOp() = default;
};
