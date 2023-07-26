#include "Exceptions.h"
#include "Expressions.h"

uint32_t Expression::ToMtemuFmt() const noexcept
{
    return _mtemuFmt;
}

Address Expression::NextAddr() const noexcept
{
    return _addr;
}

void Expression::SetAddr(const Address &addr) noexcept
{
    _addr = addr;
}

void Expression::IncrAddr(const Address &addr) noexcept
{
    _addr += addr;
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
        _mtemuFmt += etoi(BinOp::Op::SUB); // S - R - 1 + C0; C0 = 1
    }
    else
    {
        _mtemuFmt += etoi(BinOp::Op::SUB) + 1; // R - S - 1 + C0; C0 = 1
    }
    _mtemuFmt <<= 3 * WORD_SIZE;
    _mtemuFmt += in.ToMtemuFmt();
}

void BinOp::CommutativeOp(BinOp::Op opTag, const BinOpIn &in)
{
    _mtemuFmt <<= 12;
    _mtemuFmt += etoi(opTag);
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
    _mtemuFmt += etoi(shiftTag);
    _mtemuFmt <<= 3;
    _mtemuFmt += 3;
    _mtemuFmt <<= 12;
    _mtemuFmt += r.addr().value();
    _mtemuFmt <<= 4;
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
        _mtemuFmt <<= WORD_SIZE;
    }
    else
    {
        _mtemuFmt += 4;
        _mtemuFmt <<= 4;
        _mtemuFmt <<= WORD_SIZE;
        _mtemuFmt += r2.addr().value();
    }
    _mtemuFmt <<= WORD_SIZE;
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
    {
        _mtemuFmt += 3;
    }
    _mtemuFmt <<= 4;
    _mtemuFmt += 7;
    _mtemuFmt <<= 4;
    _mtemuFmt <<= WORD_SIZE * 2;
    _mtemuFmt += r.addr().value();
    _mtemuFmt <<= WORD_SIZE;

    v <<= (sizeof(v) * 8) - WORD_SIZE;
    v >>= (sizeof(v) * 8) - WORD_SIZE;
    _mtemuFmt += v;
}

void UnOp::Init(UnOp::Jmp jmpTag) noexcept
{
    _mtemuFmt = etoi(jmpTag);
    _mtemuFmt <<= 4;
    _mtemuFmt += 1;
    _mtemuFmt <<= 4;
    _mtemuFmt += 7;
    _mtemuFmt <<= 4;
    _mtemuFmt <<= 3 * WORD_SIZE;
    _mtemuFmt += 1;
}

Address UnOp::NextAddr() const noexcept
{
    return _addr + (_lbl ? _lbl->GetAddr() : 0);
}


Register BinCmd::GetExtraReg(const std::unordered_set<Register, Register::Hash> &regs)
{
    if (regs.size() >= 16)
        return Register("RQ");

    Register ans("R0");
    for (; regs.contains(ans); ans = Register::Next(ans));
    return ans;
}

BinCmd::BinCmd(BinCmd::MulCmdT, Register r1, Register r2, Register r3, Register r4)
{
    auto re1 = GetExtraReg({ r1, r2, r3, r4 });
    Register rq("RQ");

    _qexpr.push_back(std::make_unique<UnOp>(UnOp::SetOp, re1, 4));
    _qexpr.push_back(std::make_unique<BinOp>(BinOp::Op::ADD, BinOpIn(1, r4)));
    _qexpr.push_back(std::make_unique<UnOp>(UnOp::Jmp::JMP, std::make_shared<Label>("L1", 3)));
    _qexpr.push_back(std::make_unique<BinOp>(BinOp::Op::ADD, BinOpIn(rq, r3, rq)));
    _qexpr.push_back(std::make_unique<UnOp>(UnOp::Jmp::JC4, std::make_shared<Label>("L2", 7)));
    _qexpr.push_back(std::make_unique<UnOp>(UnOp::Shift::CDSRQ, r2));
    _qexpr.push_back(std::make_unique<UnOp>(UnOp::Shift::LSR, r4));
    _qexpr.push_back(std::make_unique<BinOp>(BinOp::Op::SUB, BinOpIn(re1, re1, 1)));
    _qexpr.push_back(std::make_unique<UnOp>(UnOp::Jmp::JNZ, std::make_shared<Label>("L3", 1)));
    _qexpr.push_back(std::make_unique<UnOp>(UnOp::SetOp, r1, rq));
    _qexpr.push_back(std::make_unique<UnOp>(UnOp::Jmp::JMP, std::make_shared<Label>("L4", 9)));
    _qexpr.push_back(std::make_unique<UnOp>(UnOp::Shift::CDSRQ, r2));
    _qexpr.push_back(std::make_unique<BinOp>(BinOp::Op::OR, BinOpIn(rq, 8, rq)));
    _qexpr.push_back(std::make_unique<UnOp>(UnOp::Jmp::JMP, std::make_shared<Label>("L5", 4)));
}

std::vector<Expr> &BinCmd::Get()
{
    return _qexpr;
}
