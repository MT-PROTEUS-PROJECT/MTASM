#include "Register.h"

Register::Register(std::string reg): _reg(std::move(reg)) {}

Address Register::addr() const
{
    if (isRQ())
        return {-1};
    if (_reg.size() == 2)
        return {_reg[1] - 48};
    return {(_reg[1] - 48) * 10 + (_reg[2] - 48)};
}

bool Register::isRQ() const
{
    return (_reg[1] == 'Q');
}

bool operator==(const Register &lhs, const Register &rhs)
{
    return lhs._reg == rhs._reg;
}
