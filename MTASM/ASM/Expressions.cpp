#include "Exceptions.h"
#include "Expressions.h"

ArOp::ArOp(ArOp::Op opTag, const ArOpIn &in)
{
    switch(opTag)
    {
    case ArOp::Op::ADD:
        AddOp(in);
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
    _mtemuFmt = 2;
    _mtemuFmt <<= 24;
    _mtemuFmt += in.ToMtemuFmt();
}
