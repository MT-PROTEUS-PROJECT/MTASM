#include "Address.h"

Address::Address(std::string addr) : _addr(std::stol(addr)) {}

Address::Address(AddrValue addr): _addr(addr) {}

AddrValue Address::value() const
{
    return _addr;
}
