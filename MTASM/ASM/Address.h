#pragma once

#include <string>
#include <string_view>
#include <bitset>

#include "TypeDefs.h"

class Address final
{
private:
    std::string _addr;

public:
    explicit Address(std::string addr);
    
    Address(AddrValue addr);

    template <size_t N>
    Address(const std::bitset<N> &addr): _addr(addr.to_string()) {};

    Address(const Address &) = default;
    Address &operator=(const Address &) = default;

    Address(Address &&) = default;
    Address &operator=(Address &&) = default;

    ~Address() = default;

public:
    size_t size() const;
    std::string str() const;
    std::string_view view() const;

    template <size_t N>
    std::bitset<N> bits() const
    {
        return std::bitset<N>(_addr);
    }
};
