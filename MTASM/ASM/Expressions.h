#pragma once

#include "Input.h"
#include "Label.h"

#include <queue>

class Expression
{
protected:
    uint32_t _mtemuFmt = 2; // JNXT

public:
    Expression() = default;

    virtual Address NextAddr() const noexcept;

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
private:
    std::shared_ptr<Label> _lbl;

public:
    enum Jmp
    {
        // С метками
        JNZ = 0,
        JMP = 1,
        JZ = 12,
        JF3 = 13,
        JOVR = 14,
        JC4 = 15,
        CLNZ = 4,
        CALL = 5,
        // Без меток
        JSP = 7,
        JSNZ = 8,
        JSNC4 = 11,
        RET = 6,
        PUSH = 9,
        POP = 10,
        END_LDM = 3
    };

    enum Shift
    {
        LSL = 14, // LOGICAL SHIFT LEFT
        LSR = 10, // LOGICAL SHIFT RIGHT
        CSL = 15, // CYCLICAL SHIFT LEFT
        CSR = 11, // CYCLICAL SHIFT RIGHT
        CDSL = 30, // CYCLICAL DOUBLE SHIFT LEFT
        CDSR = 26, // CYCLICAL DOUBLE SHIFT RIGHT
        ADSL = 31, // ARITHMETIC DOUBLE SHIFT LEFT
        ADSR = 27, // ARITHMETIC DOUBLE SHIFT RIGHT

        LSLQ = 12,
        LSRQ = 8,
        CSLQ = 13,
        CSRQ = 9,
        CDSLQ = 28,
        CDSRQ = 24,
        ADSLQ = 29,
        ADSRQ = 25
    };

private:
    void Init(UnOp::Jmp jmpTag) noexcept;
    
public:
    UnOp(UnOp::Jmp jmpTag, const std::shared_ptr<Label> &lbl = nullptr) noexcept;
    UnOp(UnOp::Jmp jmpTag, std::shared_ptr<Label> &&lbl) noexcept;

    UnOp(UnOp::Shift shiftTag, const Register &r) noexcept;

    Address NextAddr() const noexcept override;

    ~UnOp() = default;
};
