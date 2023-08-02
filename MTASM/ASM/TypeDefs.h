#pragma once

#include <cstdint>
#include <memory>

class Expression;

using Value = uint32_t;
using Expr = std::unique_ptr<Expression>;

constexpr uint8_t WORD_SIZE = 4;

template <typename Enumeration>
inline constexpr auto etoi(Enumeration const value) -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

namespace SE
{
    constexpr auto LBL_EXISTS = "ћетка {} уже существует!";
    constexpr auto LBL_NOT_FOUND = "ћетка {} не найдена!";
    
    constexpr auto MUL_DIFF_REG = "¬се регистры в команде умножени€ должны быть различны!";
    constexpr auto MUL_Q_REG = "»спользование регистра Q в команде умножени€ не поддерживаетс€!";

    constexpr auto BIN_3_DIFF_REG = "»спользование 3 различных регистров общего назначени€ в бинарных (микро)командах не поддерживаетс€!";
    constexpr auto BIN_1_L_R = "»спользование регистра отличного от регистра загрузки в качестве левого и правого операнда в бинарных(микро)командах не поддерживаетс€!";
    constexpr auto BIN_Q_L_R = "–егистр Q не может быть одновременно левым и правым операндом бинарной (микро)команды!";

    constexpr auto SHIFT_Q = "–егистр Q не может быть операндом сдвига";

    constexpr auto CMD_EXISTS = " оманда {} уже существует!";
    constexpr auto CMD_NOT_FOUND = " оманда {} не найдена!";
    constexpr auto CMD_INVALID_ADDR = "Ќе удалось вычислить адрес команды {}!";
}
