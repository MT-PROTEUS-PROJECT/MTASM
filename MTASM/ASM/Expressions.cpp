#include "Exceptions.h"
#include "Expressions.h"

Value Expression::ToMtemuFmt() const noexcept
{
    return _mtemuFmt;
}

Address Expression::NextAddr() const noexcept
{
    return _next_addr;
}
void Expression::SetNextAddr(const Address &addr) noexcept
{
    _next_addr = addr;
}
void Expression::IncrNextAddr(const Address &addr) noexcept
{
    _next_addr += addr;
}

Address Expression::CurAddr() const noexcept
{
    return _cur_addr;
}
void Expression::SetCurAddr(const Address& addr) noexcept
{
    _cur_addr = addr;
}
void Expression::IncrCurAddr(const Address& addr) noexcept
{
    _cur_addr += addr;
}

bool Expression::DependOnAddr() const noexcept
{
    /*
        2 - JNXT
        3 - END
        6 - RET
        7 - JSP
        9 - PUSH
        10 - POP
    */
    auto ca = GetCA();
    return !(ca == 2 || ca == 3 || ca == 6 || ca == 7 || ca == 9 || ca == 10);
}

uint8_t Expression::GetCA() const noexcept
{
    return ((_mtemuFmt & 0xF0000000) >> 28);
}

BinOp::BinOp(BinOp::Op opTag, const BinOpIn &in)
{
    switch (opTag)
    {
    case BinOp::Op::ADD:
        CommutativeOp(BinOp::Op::ADD, in);
        break;
    case BinOp::Op::ADDC:
        CommutativeOp(BinOp::Op::ADDC, in);
        break;
    case BinOp::Op::SUB:
        SubOp(in);
        break;
    case BinOp::Op::SUBC:
        SubOp(in, true);
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

void BinOp::SubOp(const BinOpIn &in, bool carry)
{
    _mtemuFmt <<= 12;
    auto regs = in.GetRegs();
    bool reverseOps = (in.GetNullPos() == 2) || (in.GetNullPos() == -1 && regs[0] && ((*regs[0].get()) == (*regs[1].get())) && !regs[2]->isRQ()) || (regs[1] && regs[1]->isRQ());
    if (reverseOps)
    {
        if (carry)
            _mtemuFmt += etoi(BinOp::Op::SUBC); // S - R - 1 + C0; C0 = 0
        else
            _mtemuFmt += etoi(BinOp::Op::SUB); // S - R - 1 + C0; C0 = 1
    }
    else
    {
        if (carry)
            _mtemuFmt += etoi(BinOp::Op::SUBC) + 1; // R - S - 1 + C0; C0 = 0
        else
            _mtemuFmt += etoi(BinOp::Op::SUB) + 1; // R - S - 1 + C0; C0 = 1
    }
    _mtemuFmt <<= 2 * ADDR_SIZE + WORD_SIZE;
    _mtemuFmt += in.ToMtemuFmt();
}

void BinOp::CommutativeOp(BinOp::Op opTag, const BinOpIn &in)
{
    _mtemuFmt <<= 12;
    _mtemuFmt += etoi(opTag);
    _mtemuFmt <<= 2 * ADDR_SIZE + WORD_SIZE;
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
    _mtemuFmt += etoi(shiftTag);
    _mtemuFmt <<= 3;
    _mtemuFmt += 3;
    _mtemuFmt <<= 4 + 2 * ADDR_SIZE;
    _mtemuFmt += r.addr().value();
    _mtemuFmt <<= WORD_SIZE;
}

UnOp::UnOp(UnOp::SetOpT, const Register &r1, const Register &r2) noexcept
{
    _mtemuFmt <<= 4;
    if (!r1.isRQ())
    {
        _mtemuFmt += 3;
    }
    _mtemuFmt <<= 4;
    if (r2.isRQ())
    {
        _mtemuFmt += 2;
        _mtemuFmt <<= 4;
        _mtemuFmt <<= ADDR_SIZE;
    }
    else
    {
        _mtemuFmt += 4;
        _mtemuFmt <<= 4;
        _mtemuFmt <<= ADDR_SIZE;
        _mtemuFmt += r2.addr().value();
    }
    _mtemuFmt <<= ADDR_SIZE;
    if (!r1.isRQ())
    {
        _mtemuFmt += r1.addr().value();
    }
    _mtemuFmt <<= WORD_SIZE;
}

UnOp::UnOp(UnOp::SetOpT, const Register &r, Value v) noexcept
{
    _mtemuFmt <<= 4;
    if (!r.isRQ())
        _mtemuFmt += 3;

    _mtemuFmt <<= 4;
    _mtemuFmt += 7;
    _mtemuFmt <<= 4;
    _mtemuFmt <<= ADDR_SIZE * 2;

    if (!r.isRQ())
        _mtemuFmt += r.addr().value();

    _mtemuFmt <<= WORD_SIZE;

    v <<= (sizeof(v) * 8) - WORD_SIZE;
    v >>= (sizeof(v) * 8) - WORD_SIZE;
    _mtemuFmt += v;
}

UnOp::UnOp(UnOp::GetOpT, const Register &r) noexcept
{
    _mtemuFmt <<= 4;
    _mtemuFmt += 1;
    _mtemuFmt <<= 4;
    if (r.isRQ())
        _mtemuFmt += 2;
    else
        _mtemuFmt += 3;
    _mtemuFmt <<= 4;
    _mtemuFmt <<= 2 * ADDR_SIZE;
    if (!r.isRQ())
        _mtemuFmt += r.addr().value();
    _mtemuFmt <<= WORD_SIZE;
}

void UnOp::Init(UnOp::Jmp jmpTag) noexcept
{
    _mtemuFmt = etoi(jmpTag);
    _mtemuFmt <<= 4;
    _mtemuFmt += 1;
    _mtemuFmt <<= 4;
    _mtemuFmt += 7;
    _mtemuFmt <<= 4;
    _mtemuFmt <<= 2 * ADDR_SIZE + WORD_SIZE;
    _mtemuFmt += 0;
}

Address UnOp::NextAddr() const noexcept
{
    return _next_addr + (_lbl ? _lbl->GetAddr() : 0);
}
