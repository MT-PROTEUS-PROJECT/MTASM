#pragma once

#include <cstdint>
#include <memory>

class Expression;

using Value = uint32_t;
using AddrValue = int32_t;
using Expr = std::unique_ptr<Expression>;

constexpr uint8_t WORD_SIZE = 4;

template <typename Enumeration>
inline constexpr auto etoi(Enumeration const value) -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}
