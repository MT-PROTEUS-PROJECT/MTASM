#include "Register.h"

size_t Register::Hash::operator()(const Register &r) const
{
    return std::hash<std::string>()(r._reg);
}

Register::Register(std::string reg): _reg(std::move(reg)) {}

Address Register::addr() const
{
    if (isRQ())
        return {Address::INVALID};
    if (_reg.size() == 2)
        return {static_cast<Address::Value>(_reg[1] - 48)};
    return {static_cast<Address::Value>((_reg[1] - 48) * 10 + (_reg[2] - 48))};
}

bool Register::isRQ() const
{
    return (_reg[1] == 'Q');
}

Register Register::Next(const Register &r)
{
    if (r.isRQ())
        return Register("RQ");
    return Register("R" + std::to_string((r.addr().value() + 1) % 16));
}

bool operator==(const Register &lhs, const Register &rhs)
{
    return lhs._reg == rhs._reg;
}
