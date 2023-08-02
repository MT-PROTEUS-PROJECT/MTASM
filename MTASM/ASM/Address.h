#pragma once

#include <string>
#include <string_view>
#include <bitset>

#include "TypeDefs.h"

#pragma warning(disable:4245) // Address::INVALID (like std::string::npos)

class Address final
{
public:
    using Value = uint32_t;
    static constexpr Value INVALID = -1;

private:
    Value _addr;

public:
    Address();
    Address(std::string addr);
    Address(Value addr);

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
    Value value() const;

    template <size_t N>
    std::bitset<N> bits() const
    {
        return std::bitset<N>(_addr);
    }
};
