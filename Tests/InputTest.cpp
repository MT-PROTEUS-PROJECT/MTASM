#include <gtest/gtest.h>
#include <array>
#include "ASM/Input.h"
#include "MemLeakDetector.h"

static consteval uint32_t ToMtemuFmt(const std::array<uint8_t, 24> &bits)
{
    uint32_t mtemu = 0;
    for (auto it = bits.cbegin(); it != bits.cend(); ++it)
    {
        mtemu <<= 1;
        mtemu += *it;
    }
    return mtemu;
}

// Test |    Case    |   M1 I6-I8 M0 I0-I2 C0 I3-I5  A    B    D   |
// -----|------------|---------------------------------------------|
// 1.1  | R1, R1, R2 |   0   011  0   001  0   000  0010 0001 0000 |
// 1.2  | R1, R2, R1 |   0   011  0   001  0   000  0010 0001 0000 |
// 1.3  | R1, R1, RQ |   0   011  0   000  0   000  0001 0001 0000 |
// 1.4  | R1, RQ, R1 |   0   011  0   000  0   000  0001 0001 0000 |
// 1.5  | RQ, R1, R2 |   0   000  0   001  0   000  0001 0010 0000 |
// 1.6  | RQ, R2, R1 |   0   000  0   001  0   000  0010 0001 0000 |
// 1.7  | RQ, RQ, R1 |   0   000  0   000  0   000  0001 0000 0000 |
// 1.8  | RQ, R1, RQ |   0   000  0   000  0   000  0001 0000 0000 |
// -----|------------|---------------------------------------------|
// 2.1  | R1, R1, 10 |   0   011  0   101  0   000  0001 0001 1010 |
// 2.2  | R1, R1, 26 |   0   011  0   101  0   000  0001 0001 1010 |
// 2.3  | R1, 10, R1 |   0   011  0   101  0   000  0001 0001 1010 |
// 2.4  | R1, R2, 10 |   0   011  0   101  0   000  0010 0001 1010 |
// 2.5  | R1, 10, R2 |   0   011  0   101  0   000  0010 0001 1010 |
// 2.6  | R1, RQ, 10 |   0   011  0   110  0   000  0000 0001 1010 |
// 2.7  | R1, 10, RQ |   0   011  0   110  0   000  0000 0001 1010 |
// 2.8  | RQ, R1, 3  |   0   000  0   101  0   000  0001 0000 0011 |
// 2.9  | RQ, 5, R1  |   0   000  0   101  0   000  0001 0000 0101 |
// 2.10 | RQ, 15, RQ |   0   000  0   110  0   000  0000 0000 1111 |
// 2.11 | RQ, RQ, 17 |   0   000  0   110  0   000  0000 0000 0001 |
// -----|------------|---------------------------------------------|
// 3.1  |   R1, R1   |   0   001  0   001  0   000  0001 0001 0000 |
// 3.2  |   R1, R2   |   0   001  0   001  0   000  0001 0010 0000 |
// 3.3  |   R1, RQ   |   0   001  0   000  0   000  0001 0000 0000 |
// 3.4  |   RQ, R1   |   0   001  0   000  0   000  0001 0000 0000 |
// -----|------------|---------------------------------------------|
// 4.1  |   R1, 128  |   0   001  0   101  0   000  0001 0000 0000 |
// 4.2  |   13, R1   |   0   001  0   101  0   000  0001 0000 1101 |
// 4.3  |   RQ, 11   |   0   001  0   110  0   000  0000 0000 1011 |
// 4.4  |   6, RQ    |   0   001  0   110  0   000  0000 0000 0110 |
// -----|------------|---------------------------------------------|

// Test 1
TEST(BinOpInput, _3Reg)
{
    MemoryLeakDetector mld;

    Register r1("R1");
    Register r2("R2");
    Register rq("RQ");
    // 1.1
    {
        BinOpIn t_1_1(r1, r1, r2);
        auto t_1_1_ans = ToMtemuFmt({ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_1_1_ans, t_1_1.ToMtemuFmt());
    }
    // 1.2
    {
        BinOpIn t_1_2(r1, r2, r1);
        auto t_1_2_ans = ToMtemuFmt({ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_1_2_ans, t_1_2.ToMtemuFmt());
    }
    // 1.3
    {
        BinOpIn t_1_3(r1, r1, rq);
        auto t_1_3_ans = ToMtemuFmt({ 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_1_3_ans, t_1_3.ToMtemuFmt());
    }
    //1.4
    {
        BinOpIn t_1_4(r1, r1, rq);
        auto t_1_4_ans = ToMtemuFmt({ 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_1_4_ans, t_1_4.ToMtemuFmt());
    }
    // 1.5
    {
        BinOpIn t_1_5(rq, r1, r2);
        auto t_1_5_ans = ToMtemuFmt({ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_1_5_ans, t_1_5.ToMtemuFmt());
    }
    // 1.6
    {
        BinOpIn t_1_6(rq, r2, r1);
        auto t_1_6_ans = ToMtemuFmt({ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_1_6_ans, t_1_6.ToMtemuFmt());
    }
    // 1.7
    {
        BinOpIn t_1_7(rq, rq, r1);
        auto t_1_7_ans = ToMtemuFmt({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_1_7_ans, t_1_7.ToMtemuFmt());
    }
    // 1.8
    {
        BinOpIn t_1_8(rq, r1, rq);
        auto t_1_8_ans = ToMtemuFmt({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_1_8_ans, t_1_8.ToMtemuFmt());
    }
}

// Test 2
TEST(BinOpInput, _2RegNum)
{
    MemoryLeakDetector mld;

    Register r1("R1");
    Register r2("R2");
    Register rq("RQ");

    // 2.1
    {
        BinOpIn t_2_1(r1, r1, 10);
        auto t_2_1_ans = ToMtemuFmt({ 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0 });
        EXPECT_EQ(t_2_1_ans, t_2_1.ToMtemuFmt());
    }
    // 2.2
    {
        BinOpIn t_2_2(r1, r1, 26);
        auto t_2_2_ans = ToMtemuFmt({ 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0 });
        EXPECT_EQ(t_2_2_ans, t_2_2.ToMtemuFmt());
    }
    // 2.3
    {
        BinOpIn t_2_3(r1, 10, r1);
        auto t_2_3_ans = ToMtemuFmt({ 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0 });
        EXPECT_EQ(t_2_3_ans, t_2_3.ToMtemuFmt());
    }
    // 2.4
    {
        BinOpIn t_2_4(r1, r2, 10);
        auto t_2_4_ans = ToMtemuFmt({ 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0 });
        EXPECT_EQ(t_2_4_ans, t_2_4.ToMtemuFmt());
    }
    // 2.5
    {
        BinOpIn t_2_5(r1, 10, r2);
        auto t_2_5_ans = ToMtemuFmt({ 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0 });
        EXPECT_EQ(t_2_5_ans, t_2_5.ToMtemuFmt());
    }
    // 2.6
    {
        BinOpIn t_2_6(r1, rq, 10);
        auto t_2_6_ans = ToMtemuFmt({ 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0 });
        EXPECT_EQ(t_2_6_ans, t_2_6.ToMtemuFmt());
    }
    // 2.7
    {
        BinOpIn t_2_7(r1, rq, 10);
        auto t_2_7_ans = ToMtemuFmt({ 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0 });
        EXPECT_EQ(t_2_7_ans, t_2_7.ToMtemuFmt());
    }
    // 2.8
    {
        BinOpIn t_2_8(rq, r1, 3);
        auto t_2_8_ans = ToMtemuFmt({ 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1 });
        EXPECT_EQ(t_2_8_ans, t_2_8.ToMtemuFmt());
    }
    // 2.9
    {
        BinOpIn t_2_9(rq, 5, r1);
        auto t_2_9_ans = ToMtemuFmt({ 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1 });
        EXPECT_EQ(t_2_9_ans, t_2_9.ToMtemuFmt());
    }
    // 2.10
    {
        BinOpIn t_2_10(rq, 15, rq);
        auto t_2_10_ans = ToMtemuFmt({ 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 });
        EXPECT_EQ(t_2_10_ans, t_2_10.ToMtemuFmt());
    }
    // 2.11
    {
        BinOpIn t_2_11(rq, rq, 17);
        auto t_2_11_ans = ToMtemuFmt({ 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_2_11_ans, t_2_11.ToMtemuFmt());
    }
}

// Test 3
TEST(BinOpInput, _2Reg)
{
    MemoryLeakDetector mld;

    Register r1("R1");
    Register r2("R2");
    Register rq("RQ");

    // 3.1
    {
        BinOpIn t_3_1(r1, r1);
        auto t_3_1_ans = ToMtemuFmt({ 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_3_1_ans, t_3_1.ToMtemuFmt());
    }
    // 3.2
    {
        BinOpIn t_3_2(r1, r2);
        auto t_3_2_ans = ToMtemuFmt({ 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_3_2_ans, t_3_2.ToMtemuFmt());
    }
    // 3.3
    {
        BinOpIn t_3_3(r1, rq);
        auto t_3_3_ans = ToMtemuFmt({ 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_3_3_ans, t_3_3.ToMtemuFmt());
    }
    // 3.4
    {
        BinOpIn t_3_4(rq, r1);
        auto t_3_4_ans = ToMtemuFmt({ 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_3_4_ans, t_3_4.ToMtemuFmt());
    }
}

// Test 4
TEST(BinOpInput, _RegNum)
{
    MemoryLeakDetector mld;

    Register r1("R1");
    Register rq("RQ");

    // 4.1
    {
        BinOpIn t_4_1(r1, 128);
        auto t_4_1_ans = ToMtemuFmt({ 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_4_1_ans, t_4_1.ToMtemuFmt());
    }
    // 4.2
    {
        BinOpIn t_4_2(13, r1);
        auto t_4_2_ans = ToMtemuFmt({ 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1 });
        EXPECT_EQ(t_4_2_ans, t_4_2.ToMtemuFmt());
    }
    // 4.3
    {
        BinOpIn t_4_3(rq, 11);
        auto t_4_3_ans = ToMtemuFmt({ 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1 });
        EXPECT_EQ(t_4_3_ans, t_4_3.ToMtemuFmt());
    }
    // 4.4
    {
        BinOpIn t_4_4(6, rq);
        auto t_4_4_ans = ToMtemuFmt({ 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0 });
        EXPECT_EQ(t_4_4_ans, t_4_4.ToMtemuFmt());
    }
}
