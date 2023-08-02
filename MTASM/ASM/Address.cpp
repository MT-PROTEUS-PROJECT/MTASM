#include "Address.h"

Address::Address() : _addr(0) {}

Address::Address(std::string addr) : _addr(std::stol(addr)) {}

Address::Address(Address::Value addr) : _addr(addr) {}

Address &Address::operator+=(const Address &rhs)
{
    _addr += rhs._addr;
    return *this;
}

Address operator+(const Address &lhs, const Address &rhs)
{
    return { lhs._addr + rhs._addr };
}

Address::Value Address::value() const
{
    return _addr;
}
