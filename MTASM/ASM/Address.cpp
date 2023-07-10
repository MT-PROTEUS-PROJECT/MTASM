#include "Address.h"

Address::Address(std::string addr) : _addr(std::move(addr)) {}

Address::Address(AddrValue addr): _addr(std::to_string(addr)) {}

size_t Address::size() const
{
    return _addr.size();
}

std::string Address::str() const
{
    return _addr;
}

std::string_view Address::view() const
{
    return { _addr.data(), _addr.size() };
}
