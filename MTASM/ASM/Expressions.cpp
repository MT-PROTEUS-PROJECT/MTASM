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

    _in = std::make_shared<BinOpIn>(in);
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

void BinOp::SpecTemplates(const std::vector<Register>& args, const std::vector<Register>& spec)
{
    auto regs = _in->GetRegs();
    for (auto& reg : regs)
    {
        if (!reg || !reg->isTemplate())
            continue;
        for (size_t i = 0; i < args.size(); ++i)
        {
            if (*reg == args[i])
            {
                reg = std::make_shared<Register>(spec[i]);
                break;
            }
        }
    }
    _in->Update(regs);
    _mtemuFmt += _in->ToMtemuFmt();
}

bool BinOp::CheckTemplates(const std::vector<Register>& templates) const
{
    auto _templates = _in->GetTemplateRegs();
    for (const auto& t : _templates)
    {
        bool found = false;
        if (!t)
            continue;
        for (const auto& tt : templates)
        {
            if (tt == *t.get())
            {
                found = true;
                break;
            }
        }

        if (!found)
            return false;
    }

    return true;
}

UnOp::UnOp(UnOp::Jmp jmpTag, const std::shared_ptr<Label> &lbl, bool isCmdJmp) noexcept : _lbl(lbl), _isCmdJmp(isCmdJmp), _type(Type::JMP)
{
    tag = etoi(jmpTag);
    calcMtemuJmp();
}

UnOp::UnOp(UnOp::Jmp jmpTag, std::shared_ptr<Label> &&lbl, bool isCmdJmp) noexcept : _lbl(std::move(lbl)), _isCmdJmp(isCmdJmp), _type(Type::JMP)
{
    tag = etoi(jmpTag);
    calcMtemuJmp();
}

UnOp::UnOp(UnOp::Shift shiftTag, const Register &r) noexcept : _type(Type::SHIFT)
{
    _in.regs.push_back(r);
    tag = etoi(shiftTag);
    calcMtemuShift();
}

UnOp::UnOp(UnOp::SetOpT, const Register &r1, const Register &r2) noexcept : _type(Type::SET)
{
    _in.regs.push_back(r1);
    _in.regs.push_back(r2);
    calcMtemuSet();
}

UnOp::UnOp(UnOp::SetOpT, const Register &r, Value v) noexcept : _type(Type::SET)
{
    _in.regs.push_back(r);
    _in.v = v;
    calcMtemuSet();
}

UnOp::UnOp(UnOp::GetOpT, const Register &r) noexcept : _type(Type::GET)
{
    _in.regs.push_back(r);
    calcMtemuGet();
}

void UnOp::calcMtemuJmp()
{
    _mtemuFmt = tag;
    _mtemuFmt <<= 4;
    _mtemuFmt += 1;
    _mtemuFmt <<= 4;
    _mtemuFmt += 7;
    _mtemuFmt <<= 4;
    _mtemuFmt <<= 2 * ADDR_SIZE + WORD_SIZE;
    _mtemuFmt += 0;
}

void UnOp::calcMtemuShift()
{
    _mtemuFmt = 2;
    _mtemuFmt <<= 5;
    _mtemuFmt += tag;
    _mtemuFmt <<= 3;
    _mtemuFmt += 3;
    _mtemuFmt <<= 4 + 2 * ADDR_SIZE;
    if (!_in.regs[0].isTemplate())
    {
        _mtemuFmt += _in.regs[0].addr().value();
        _mtemuFmt <<= WORD_SIZE;
    }
}

void UnOp::calcMtemuGet()
{
    auto r = _in.regs[0];
    if (r.isTemplate())
        return;
    _mtemuFmt = 2;
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

void UnOp::calcMtemuSet()
{
    if (_in.regs.size() == 2)
    {
        Register r1 = _in.regs[0];
        Register r2 = _in.regs[1];
        if (r1.isTemplate() || r2.isTemplate())
            return;
        _mtemuFmt = 2;
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
    else
    {
        Register r = _in.regs[0];
        Value v = _in.v;

        if (r.isTemplate())
            return;
        _mtemuFmt = 2;
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
}

Address UnOp::NextAddr() const noexcept
{
    if (_isCmdJmp)
    {
        return _lbl ? _lbl->GetAddr() : 0;
    }
    return _next_addr + (_lbl ? _lbl->GetAddr() : 0);
}

void UnOp::SetNextAddr(const Address& addr) noexcept
{
    if (_lbl)
    {
        _next_addr = 0;
        _lbl->SetAddr(addr);
    }
    else
    {
        _next_addr = addr;
    }
}

void UnOp::SpecTemplates(const std::vector<Register>& args, const std::vector<Register>& spec)
{
    for (auto& reg : _in.regs)
    {
        for (size_t i = 0; i < args.size(); ++i)
        {
            if (reg == args[i])
            {
                reg = Register(spec[i]);
                break;
            }
        }
    }

    switch (_type)
    {
    case Type::JMP:
        calcMtemuJmp();
        break;
    case Type::SHIFT:
        calcMtemuShift();
        break;
    case Type::SET:
        calcMtemuSet();
        break;
    case Type::GET:
        calcMtemuGet();
        break;
    }
}

bool UnOp::CheckTemplates(const std::vector<Register>& templates) const
{
    for (const auto& t : _in.regs)
    {
        if (!t.isTemplate())
            continue;
        bool found = false;
        for (const auto& tt : templates)
        {
            if (tt == t)
            {
                found = true;
                break;
            }
        }

        if (!found)
            return false;
    }

    return true;
}
