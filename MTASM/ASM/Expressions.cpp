#include "Exceptions.h"
#include "Expressions.h"

uint32_t Expression::ToMtemuFmt() const noexcept
{
    return _mtemuFmt;
}

Address Expression::NextAddr() const noexcept
{
    return { 0 };
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
    default:
        throw InternalCompilerError("Неизвестная арифметическая микрокоманда!");
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

UnOp::UnOp(UnOp::Jmp jmpTag, const std::shared_ptr<Label> &lbl) noexcept : _lbl(lbl)
{
    Init(jmpTag);
}

UnOp::UnOp(UnOp::Jmp jmpTag, std::shared_ptr<Label> &&lbl) noexcept : _lbl(std::move(lbl))
{
    Init(jmpTag);
}

UnOp::UnOp(UnOp::Shift shiftTag, const Register &r) noexcept
{
    _mtemuFmt <<= 5;
    _mtemuFmt += shiftTag;
    _mtemuFmt <<= 3;
    _mtemuFmt += 3;
    _mtemuFmt <<= 12;
    _mtemuFmt += r.addr().value();
    _mtemuFmt <<= 4;
}

void UnOp::Init(UnOp::Jmp jmpTag) noexcept
{
    _mtemuFmt = jmpTag;
    _mtemuFmt <<= 24;
}

Address UnOp::NextAddr() const noexcept
{
    return _lbl ? _lbl->GetAddr() : 0;
}
