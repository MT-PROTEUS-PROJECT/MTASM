#pragma once

#include <string>
#include <string_view>
#include <bitset>

#include "TypeDefs.h"

class Address final
{
private:
    AddrValue _addr;

public:
    Address();
    Address(std::string addr);
    Address(AddrValue addr);

    template <size_t N>
    Address(const std::bitset<N> &addr): _addr(addr.to_ulong()) {};

    Address(const Address &) = default;
    Address &operator=(const Address &) = default;

    Address(Address &&) = default;
    Address &operator=(Address &&) = default;

    Address &operator+=(const Address &rhs);
    friend Address operator+(const Address &lhs, const Address &rhs);

    ~Address() = default;

public:
    AddrValue value() const;

    template <size_t N>
    std::bitset<N> bits() const
    {
        return std::bitset<N>(_addr);
    }
};
