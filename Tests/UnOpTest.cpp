#include <gtest/gtest.h>
#include <array>
#include "ASM/Expressions.h"
#include "MemLeakDetector.h"

static consteval uint32_t ToMtemuFmt(const std::array<uint8_t, 28> &bits)
{
    uint32_t mtemu = 0;
    for (auto it = bits.cbegin(); it != bits.cend(); ++it)
    {
        mtemu <<= 1;
        mtemu += *it;
    }
    return mtemu;
}

// Test |    Case        |  CA  M1 I6-I8 M0 I0-I2 C0 I3-I5   A    B    D   |
// -----|----------------|-------------------------------------------------|
// 1.1  |      JNZ       | 0000  0   001  0   111  0   000  0000 0000 0001 |
// 1.2  |      JMP       | 0001  0   001  0   111  0   000  0000 0000 0001 |
// 1.3  |    END/LDM     | 0011  0   001  0   111  0   000  0000 0000 0001 |
// 1.4  |     CLNZ       | 0100  0   001  0   111  0   000  0000 0000 0001 |
// 1.5  |     CALL       | 0101  0   001  0   111  0   000  0000 0000 0001 |
// 1.6  |      RET       | 0110  0   001  0   111  0   000  0000 0000 0001 | 
// 1.7  |      JSP       | 0111  0   001  0   111  0   000  0000 0000 0001 |
// 1.8  |     JSNZ       | 1000  0   001  0   111  0   000  0000 0000 0001 |
// 1.9  |     PUSH       | 1001  0   001  0   111  0   000  0000 0000 0001 |
// 1.10 |      POP       | 1010  0   001  0   111  0   000  0000 0000 0001 |
// 1.11 |     JSNC4      | 1011  0   001  0   111  0   000  0000 0000 0001 |
// 1.12 |      JZ        | 1100  0   001  0   111  0   000  0000 0000 0001 |
// 1.13 |      JF3       | 1101  0   001  0   111  0   000  0000 0000 0001 |
// 1.14 |      JOVR      | 1110  0   001  0   111  0   000  0000 0000 0001 |
// 1.15 |      JC4       | 1111  0   001  0   111  0   000  0000 0000 0001 |
// -----|----------------|-------------------------------------------------|
// 2.1  |     LSL R0     | 0010  0   111  0   011  0   000  0000 0000 0000 |
// 2.2  |     LSR R1     | 0010  0   101  0   011  0   000  0000 0001 0000 |
// 2.3  |     CSL R2     | 0010  0   111  1   011  0   000  0000 0010 0000 |
// 2.4  |     CSR R3     | 0010  0   101  1   011  0   000  0000 0011 0000 |
// 2.5  |     CDSL R4    | 0010  1   111  0   011  0   000  0000 0100 0000 |
// 2.6  |     CDSR R5    | 0010  1   101  0   011  0   000  0000 0101 0000 |
// 2.7  |     ADSL R6    | 0010  1   111  1   011  0   000  0000 0110 0000 |
// 2.8  |     ADSR R7    | 0010  1   101  1   011  0   000  0000 0111 0000 |
// 2.9  |     LSLQ R8    | 0010  0   110  0   011  0   000  0000 1000 0000 |
// 2.10 |     LSRQ R9    | 0010  0   100  0   011  0   000  0000 1001 0000 |
// 2.11 |     CSLQ R10   | 0010  0   110  1   011  0   000  0000 1010 0000 |
// 2.12 |     CSRQ R11   | 0010  0   100  1   011  0   000  0000 1011 0000 |
// 2.13 |     CDSLQ R12  | 0010  1   110  0   011  0   000  0000 1100 0000 |
// 2.14 |     CDSRQ R13  | 0010  1   100  0   011  0   000  0000 1101 0000 |
// 2.15 |     ADSLQ R14  | 0010  1   110  1   011  0   000  0000 1110 0000 |
// 2.16 |     ADSRQ R15  | 0010  1   100  1   011  0   000  0000 1111 0000 |
// -----|----------------|-------------------------------------------------|

// Test 1
TEST(UnOp, _Jmp)
{
    MemoryLeakDetector mld;

    // 1.1
    {
        UnOp t_1_1(UnOp::Jmp::JNZ);
        auto t_1_1_ans = ToMtemuFmt({ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_1_ans, t_1_1.ToMtemuFmt());
    }
    // 1.2
    {
        UnOp t_1_2(UnOp::Jmp::JMP);
        auto t_1_2_ans = ToMtemuFmt({ 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_2_ans, t_1_2.ToMtemuFmt());
    }
    // 1.3
    {
        UnOp t_1_3(UnOp::Jmp::END_LDM);
        auto t_1_3_ans = ToMtemuFmt({ 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_3_ans, t_1_3.ToMtemuFmt());
    }
    // 1.4
    {
        UnOp t_1_4(UnOp::Jmp::CLNZ);
        auto t_1_4_ans = ToMtemuFmt({ 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_4_ans, t_1_4.ToMtemuFmt());
    }
    // 1.5
    {
        UnOp t_1_5(UnOp::Jmp::CALL);
        auto t_1_5_ans = ToMtemuFmt({ 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_5_ans, t_1_5.ToMtemuFmt());
    }
    // 1.6
    {
        UnOp t_1_6(UnOp::Jmp::RET);
        auto t_1_6_ans = ToMtemuFmt({ 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_6_ans, t_1_6.ToMtemuFmt());
    }
    // 1.7
    {
        UnOp t_1_7(UnOp::Jmp::JSP);
        auto t_1_7_ans = ToMtemuFmt({ 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_7_ans, t_1_7.ToMtemuFmt());
    }
    // 1.8
    {
        UnOp t_1_8(UnOp::Jmp::JSNZ);
        auto t_1_8_ans = ToMtemuFmt({ 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_8_ans, t_1_8.ToMtemuFmt());
    }
    // 1.9
    {
        UnOp t_1_9(UnOp::Jmp::PUSH);
        auto t_1_9_ans = ToMtemuFmt({ 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_9_ans, t_1_9.ToMtemuFmt());
    }
    // 1.10
    {
        UnOp t_1_10(UnOp::Jmp::POP);
        auto t_1_10_ans = ToMtemuFmt({ 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_10_ans, t_1_10.ToMtemuFmt());
    }
    // 1.11
    {
        UnOp t_1_11(UnOp::Jmp::JSNC4);
        auto t_1_11_ans = ToMtemuFmt({ 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_11_ans, t_1_11.ToMtemuFmt());
    }
    // 1.12
    {
        UnOp t_1_12(UnOp::Jmp::JZ);
        auto t_1_12_ans = ToMtemuFmt({ 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_12_ans, t_1_12.ToMtemuFmt());
    }
    // 1.13
    {
        UnOp t_1_13(UnOp::Jmp::JF3);
        auto t_1_13_ans = ToMtemuFmt({ 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_13_ans, t_1_13.ToMtemuFmt());
    }
    // 1.14
    {
        UnOp t_1_14(UnOp::Jmp::JOVR);
        auto t_1_14_ans = ToMtemuFmt({ 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_14_ans, t_1_14.ToMtemuFmt());
    }
    // 1.15
    {
        UnOp t_1_15(UnOp::Jmp::JC4);
        auto t_1_15_ans = ToMtemuFmt({ 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_1_15_ans, t_1_15.ToMtemuFmt());
    }
}

// Test 2
TEST(UnOp, _Shift)
{
    MemoryLeakDetector mld;

    // 2.1
    {
        UnOp t_2_1(UnOp::Shift::LSL, Register("R0"));
        auto t_2_1_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_1_ans, t_2_1.ToMtemuFmt());
    }
    // 2.2
    {
        UnOp t_2_2(UnOp::Shift::LSR, Register("R1"));
        auto t_2_2_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_2_ans, t_2_2.ToMtemuFmt());
    }
    // 2.3
    {
        UnOp t_2_3(UnOp::Shift::CSL, Register("R2"));
        auto t_2_3_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_3_ans, t_2_3.ToMtemuFmt());
    }
    // 2.4
    {
        UnOp t_2_4(UnOp::Shift::CSR, Register("R3"));
        auto t_2_4_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_4_ans, t_2_4.ToMtemuFmt());
    }
    // 2.5
    {
        UnOp t_2_5(UnOp::Shift::CDSL, Register("R4"));
        auto t_2_5_ans = ToMtemuFmt({ 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_5_ans, t_2_5.ToMtemuFmt());
    }
    // 2.6
    {
        UnOp t_2_6(UnOp::Shift::CDSR, Register("R5"));
        auto t_2_6_ans = ToMtemuFmt({ 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_6_ans, t_2_6.ToMtemuFmt());
    }
    // 2.7
    {
        UnOp t_2_7(UnOp::Shift::ADSL, Register("R6"));
        auto t_2_7_ans = ToMtemuFmt({ 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_7_ans, t_2_7.ToMtemuFmt());
    }
    // 2.8
    {
        UnOp t_2_8(UnOp::Shift::ADSR, Register("R7"));
        auto t_2_8_ans = ToMtemuFmt({ 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_8_ans, t_2_8.ToMtemuFmt());
    }
    // 2.9
    {
        UnOp t_2_9(UnOp::Shift::LSLQ, Register("R8"));
        auto t_2_9_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_9_ans, t_2_9.ToMtemuFmt());
    }
    // 2.10
    {
        UnOp t_2_10(UnOp::Shift::LSRQ, Register("R9"));
        auto t_2_10_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_10_ans, t_2_10.ToMtemuFmt());
    }
    // 2.11
    {
        UnOp t_2_11(UnOp::Shift::CSLQ, Register("R10"));
        auto t_2_11_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_11_ans, t_2_11.ToMtemuFmt());
    }
    // 2.12
    {
        UnOp t_2_12(UnOp::Shift::CSRQ, Register("R11"));
        auto t_2_12_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_12_ans, t_2_12.ToMtemuFmt());
    }
    // 2.13
    {
        UnOp t_2_13(UnOp::Shift::CDSLQ, Register("R12"));
        auto t_2_13_ans = ToMtemuFmt({ 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_13_ans, t_2_13.ToMtemuFmt());
    }
    // 2.14
    {
        UnOp t_2_14(UnOp::Shift::CDSRQ, Register("R13"));
        auto t_2_14_ans = ToMtemuFmt({ 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_14_ans, t_2_14.ToMtemuFmt());
    }
    // 2.15
    {
        UnOp t_2_15(UnOp::Shift::ADSLQ, Register("R14"));
        auto t_2_15_ans = ToMtemuFmt({ 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_15_ans, t_2_15.ToMtemuFmt());
    }
    // 2.16
    {
        UnOp t_2_16(UnOp::Shift::ADSRQ, Register("R15"));
        auto t_2_16_ans = ToMtemuFmt({ 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_16_ans, t_2_16.ToMtemuFmt());
    }
}
