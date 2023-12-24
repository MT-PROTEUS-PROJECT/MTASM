#pragma once

#include <cstdint>
#include <memory>

class Expression;

using Value = uint32_t;
using Expr = std::shared_ptr<Expression>;

constexpr uint8_t ADDR_SIZE = 4;
constexpr uint8_t WORD_SIZE = 8;

template <typename Enumeration>
inline constexpr auto etoi(Enumeration const value) -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

namespace SE
{
    constexpr auto LBL_EXISTS = "ћетка {} уже существует!";
    constexpr auto LBL_NOT_FOUND = "ћетка {} не найдена!";
    
    constexpr auto BIN_3_DIFF_REG = "»спользование 3 различных регистров общего назначени€ в бинарных (микро)командах не поддерживаетс€!";
    constexpr auto BIN_1_L_R = "»спользование регистра отличного от регистра загрузки в качестве левого и правого операнда в бинарных(микро)командах не поддерживаетс€!";
    constexpr auto BIN_Q_L_R = "–егистр Q не может быть одновременно левым и правым операндом бинарной (микро)команды!";

    constexpr auto SHIFT_Q = "–егистр Q не может быть операндом сдвига!";

    constexpr auto CMD_EXISTS = " оманда {} уже существует!";
    constexpr auto CMD_NOT_FOUND = " оманда {} не найдена!";
    constexpr auto CMD_INVALID_ADDR = "Ќе удалось вычислить адрес команды {}!";

    constexpr auto CMD_INVALID_ARGS = "јргументы команды должны иметь вид R[a-z]!";
    constexpr auto CMD_SAME_ARGS = "јргументы команды должны иметь разные наименовани€!";
    constexpr auto CMD_INVALID_INSTANCE = "ѕри вызове функции передаваемые параметры должны иметь вид R[0-15] или RQ!";
    constexpr auto CMD_HAS_NOT_SPEC_ARGS = " оманда {} не содержит аргументы, используемые в микрокоманде!";
    constexpr auto CMD_INVALID_ARGS_COUNT = "Ќеверное число аргументов при вызове команды!";
}
