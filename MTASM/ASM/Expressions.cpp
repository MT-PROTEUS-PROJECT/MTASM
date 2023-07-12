#include "Exceptions.h"
#include "Expressions.h"

ArOp::ArOp(ArOp::Op opTag, const ArOpIn &in)
{
    switch(opTag)
    {
    case ArOp::Op::ADD:
        AddOp(in);
        break;
    case ArOp::Op::SUB:
        SubOp(in);
        break;
    default:
        throw InternalCompilerError("Арифметическая операция не поддерживается!");
    }
}

uint32_t ArOp::ToMtemuFmt() const
{
    return _mtemuFmt;
}

void ArOp::AddOp(const ArOpIn &in)
{
    _mtemuFmt <<= 24;
    _mtemuFmt += in.ToMtemuFmt();
}

void ArOp::SubOp(const ArOpIn &in)
{
    _mtemuFmt <<= 12;
    auto regs = in.GetRegs();
    bool reverseOps = (in.GetNullPos() == 2) || (in.GetNullPos() == -1 && regs[0] && ((*regs[0].get()) == (*regs[1].get())) && !regs[2]->isRQ()) || (regs[1] && regs[1]->isRQ());
    if (reverseOps)
    {
        _mtemuFmt += 9; // S - R - 1 + C0; C0 = 1
    }
    else
    {
        _mtemuFmt += 10; // R - S - 1 + C0; C0 = 1
    }
    _mtemuFmt <<= 3 * WORD_SIZE;
    _mtemuFmt += in.ToMtemuFmt();
}
