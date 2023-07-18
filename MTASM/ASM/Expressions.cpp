#include "Exceptions.h"
#include "Expressions.h"

uint32_t Expression::ToMtemuFmt() const noexcept
{
    return _mtemuFmt;
}

BinOp::BinOp(BinOp::Op opTag, const BinOpIn &in)
{
    switch (opTag)
    {
    case BinOp::Op::ADD:
        CommutativeOp(BinOp::Op::ADD, in);
        break;
    case BinOp::Op::SUB:
        SubOp(in);
        break;
    case BinOp::Op::OR:
        CommutativeOp(BinOp::Op::OR, in);
        break;
    case BinOp::Op::AND:
        CommutativeOp(BinOp::Op::AND, in);
        break;
    case BinOp::Op::XOR:
        CommutativeOp(BinOp::Op::XOR, in);
        break;
    case BinOp::Op::NXOR:
        CommutativeOp(BinOp::Op::NXOR, in);
        break;
    case BinOp::Op::MUL:
    case BinOp::Op::DIV:
    default:
        throw InternalCompilerError("Арифметическая операция не поддерживается!");
    }
}

void BinOp::SubOp(const BinOpIn &in)
{
    _mtemuFmt <<= 12;
    auto regs = in.GetRegs();
    bool reverseOps = (in.GetNullPos() == 2) || (in.GetNullPos() == -1 && regs[0] && ((*regs[0].get()) == (*regs[1].get())) && !regs[2]->isRQ()) || (regs[1] && regs[1]->isRQ());
    if (reverseOps)
    {
        _mtemuFmt += BinOp::Op::SUB; // S - R - 1 + C0; C0 = 1
    }
    else
    {
        _mtemuFmt += BinOp::Op::SUB + 1; // R - S - 1 + C0; C0 = 1
    }
    _mtemuFmt <<= 3 * WORD_SIZE;
    _mtemuFmt += in.ToMtemuFmt();
}

void BinOp::CommutativeOp(BinOp::Op opTag, const BinOpIn &in)
{
    _mtemuFmt <<= 12;
    _mtemuFmt += opTag;
    _mtemuFmt <<= 3 * WORD_SIZE;
    _mtemuFmt += in.ToMtemuFmt();
}

UnOp::UnOp(UnOp::Op opTag, const Label &lbl) : _lbl(lbl)
{
    _mtemuFmt = opTag;
    _mtemuFmt <<= 24;
}
