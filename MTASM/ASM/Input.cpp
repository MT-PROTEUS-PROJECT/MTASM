#include "Input.h"

ArOpIn::ArOpIn(Register r1, Register r2, Register r3) : _load(true), _value(0)
{
    _regs.push_back(std::make_unique<Register>(std::move(r1)));
    _regs.push_back(std::make_unique<Register>(std::move(r2)));
    _regs.push_back(std::make_unique<Register>(std::move(r3)));
}

ArOpIn::ArOpIn(Register r1, Register r2, Value v) : _load(true), _value(v)
{
    _regs.push_back(std::make_unique<Register>(std::move(r1)));
    _regs.push_back(std::make_unique<Register>(std::move(r2)));
    _regs.push_back(nullptr);
}

ArOpIn::ArOpIn(Register r1, Value v, Register r2) : _load(true), _value(v)
{
    _regs.push_back(std::make_unique<Register>(std::move(r1)));
    _regs.push_back(nullptr);
    _regs.push_back(std::make_unique<Register>(std::move(r2)));
}

ArOpIn::ArOpIn(Register r1, Register r2): _load(false), _value(0)
{
    _regs.push_back(std::make_unique<Register>(std::move(r1)));
    _regs.push_back(std::make_unique<Register>(std::move(r2)));
    _regs.push_back(nullptr);
}

ArOpIn::ArOpIn(Register r1, Value v): _load(false), _value(v)
{
    _regs.push_back(std::make_unique<Register>(std::move(r1)));
    _regs.push_back(nullptr);
    _regs.push_back(nullptr);
}

ArOpIn::ArOpIn(Value v, Register r1): _load(false), _value(v)
{
    _regs.push_back(nullptr);
    _regs.push_back(std::make_unique<Register>(std::move(r1)));
    _regs.push_back(nullptr);
}
