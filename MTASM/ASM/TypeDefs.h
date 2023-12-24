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
    constexpr auto LBL_EXISTS = "����� {} ��� ����������!";
    constexpr auto LBL_NOT_FOUND = "����� {} �� �������!";
    
    constexpr auto BIN_3_DIFF_REG = "������������� 3 ��������� ��������� ������ ���������� � �������� (�����)�������� �� ��������������!";
    constexpr auto BIN_1_L_R = "������������� �������� ��������� �� �������� �������� � �������� ������ � ������� �������� � ��������(�����)�������� �� ��������������!";
    constexpr auto BIN_Q_L_R = "������� Q �� ����� ���� ������������ ����� � ������ ��������� �������� (�����)�������!";

    constexpr auto SHIFT_Q = "������� Q �� ����� ���� ��������� ������!";

    constexpr auto CMD_EXISTS = "������� {} ��� ����������!";
    constexpr auto CMD_NOT_FOUND = "������� {} �� �������!";
    constexpr auto CMD_INVALID_ADDR = "�� ������� ��������� ����� ������� {}!";

    constexpr auto CMD_INVALID_ARGS = "��������� ������� ������ ����� ��� R[a-z]!";
    constexpr auto CMD_SAME_ARGS = "��������� ������� ������ ����� ������ ������������!";
    constexpr auto CMD_INVALID_INSTANCE = "��� ������ ������� ������������ ��������� ������ ����� ��� R[0-15] ��� RQ!";
    constexpr auto CMD_HAS_NOT_SPEC_ARGS = "������� {} �� �������� ���������, ������������ � ������������!";
    constexpr auto CMD_INVALID_ARGS_COUNT = "�������� ����� ���������� ��� ������ �������!";
}
