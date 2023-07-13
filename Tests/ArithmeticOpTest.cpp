#include <gtest/gtest.h>
#include <array>
#include "ASM/Expressions.h"

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
// 1.1  | ADD RQ, R2, 30 | 0010  0   000  0   101  0   000  0010 0000 1110 |
// -----|----------------|-------------------------------------------------|
// 2.1  | SUB R1, R1, R2 | 0010  0   011  0   001  1   001  0010 0001 0000 |
// 2.2  | SUB R1, R2, R1 | 0010  0   011  0   001  1   010  0010 0001 0000 |
// 2.3  | SUB R1, R1, RQ | 0010  0   011  0   000  1   010  0001 0001 0000 |
// 2.4  | SUB R1, RQ, R1 | 0010  0   011  0   000  1   001  0001 0001 0000 |
// 2.5  | SUB RQ, R1, R2 | 0010  0   000  0   001  1   010  0001 0010 0000 |
// 2.6  | SUB RQ, RQ, R1 | 0010  0   000  0   000  1   001  0001 0000 0000 |
// 2.7  | SUB RQ, R1, RQ | 0010  0   000  0   000  1   010  0001 0000 0000 |
// -----|----------------|-------------------------------------------------|
// 3.1  | SUB R1, R1, 10 | 0010  0   011  0   101  1   001  0001 0001 1010 |
// 3.2  | SUB R1, 10, R1 | 0010  0   011  0   101  1   010  0001 0001 1010 |
// 3.3  | SUB R1, RQ, 10 | 0010  0   011  0   110  1   001  0000 0001 1010 |
// 3.4  | SUB R1, 10, RQ | 0010  0   011  0   110  1   010  0000 0001 1010 |
// 3.5  | SUB RQ, R1, 3  | 0010  0   000  0   101  1   001  0001 0000 0011 |
// 3.6  | SUB RQ, 5, R1  | 0010  0   000  0   101  1   010  0001 0000 0101 |
// 3.7  | SUB RQ, 15, RQ | 0010  0   000  0   110  1   010  0000 0000 1111 |
// 3.8  | SUB RQ, RQ, 17 | 0010  0   000  0   110  1   001  0000 0000 0001 |
// -----|----------------|-------------------------------------------------|
// 4.1  |   SUB R1, R1   | 0010  0   001  0   001  1   010  0001 0001 0000 |
// 4.2  |   SUB R1, R2   | 0010  0   001  0   001  1   010  0001 0010 0000 |
// 4.3  |   SUB R1, RQ   | 0010  0   001  0   000  1   010  0001 0000 0000 |
// 4.4  |   SUB RQ, R1   | 0010  0   001  0   000  1   001  0001 0000 0000 |
// -----|----------------|-------------------------------------------------|
// 5.1  |   SUB R1, 128  | 0010  0   001  0   101  1   001  0001 0000 0000 |
// 5.2  |   SUB 13, R1   | 0010  0   001  0   101  1   010  0001 0000 1101 |
// 5.3  |   SUB RQ, 11   | 0010  0   001  0   110  1   001  0000 0000 1011 |
// 5.4  |   SUB 6, RQ    | 0010  0   001  0   110  1   010  0000 0000 0110 |
// -----|----------------|-------------------------------------------------|


// Test 1
TEST(ArithmeticOp, _Add)
{
    Register r2("R2");
    Register rq("RQ");
    // 1.1
    {
        BinOp t_1_1(BinOp::Op::ADD, BinOpIn(rq, r2, 30));
        auto t_1_1_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0 });
        EXPECT_EQ(t_1_1_ans, t_1_1.ToMtemuFmt());
    }
}

// Test 2-4
TEST(ArithmeticOp, _Sub)
{
    Register r1("R1");
    Register r2("R2");
    Register rq("RQ");
    // 2.1
    {
        BinOp t_2_1(BinOp::Op::SUB, BinOpIn(r1, r1, r2));
        auto t_2_1_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_1_ans, t_2_1.ToMtemuFmt());
    }
    // 2.2
    {
        BinOp t_2_2(BinOp::Op::SUB, BinOpIn(r1, r2, r1));
        auto t_2_2_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_2_ans, t_2_2.ToMtemuFmt());
    }
    // 2.3
    {
        BinOp t_2_3(BinOp::Op::SUB, BinOpIn(r1, r1, rq));
        auto t_2_3_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_3_ans, t_2_3.ToMtemuFmt());
    }
    // 2.4
    {
        BinOp t_2_4(BinOp::Op::SUB, BinOpIn(r1, rq, r1));
        auto t_2_4_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_4_ans, t_2_4.ToMtemuFmt());
    }
    // 2.5
    {
        BinOp t_2_5(BinOp::Op::SUB, BinOpIn(rq, r1, r2));
        auto t_2_5_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_5_ans, t_2_5.ToMtemuFmt());
    }
    // 2.6
    {
        BinOp t_2_6(BinOp::Op::SUB, BinOpIn(rq, rq, r1));
        auto t_2_6_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_6_ans, t_2_6.ToMtemuFmt());
    }
    // 2.7
    {
        BinOp t_2_7(BinOp::Op::SUB, BinOpIn(rq, r1, rq));
        auto t_2_7_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_2_7_ans, t_2_7.ToMtemuFmt());
    }
    // 3.1
    {
        BinOp t_3_1(BinOp::Op::SUB, BinOpIn(r1, r1, 10));
        auto t_3_1_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0 });
        EXPECT_EQ(t_3_1_ans, t_3_1.ToMtemuFmt());
    }
    // 3.2
    {
        BinOp t_3_2(BinOp::Op::SUB, BinOpIn(r1, 10, r1));
        auto t_3_2_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0 });
        EXPECT_EQ(t_3_2_ans, t_3_2.ToMtemuFmt());
    }
    // 3.3
    {
        BinOp t_3_3(BinOp::Op::SUB, BinOpIn(r1, rq, 10));
        auto t_3_3_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0 });
        EXPECT_EQ(t_3_3_ans, t_3_3.ToMtemuFmt());
    }
    // 3.4
    {
        BinOp t_3_4(BinOp::Op::SUB, BinOpIn(r1, 10, rq));
        auto t_3_4_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0 });
        EXPECT_EQ(t_3_4_ans, t_3_4.ToMtemuFmt());
    }
    // 3.5
    {
        BinOp t_3_5(BinOp::Op::SUB, BinOpIn(rq, r1, 3));
        auto t_3_5_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1 });
        EXPECT_EQ(t_3_5_ans, t_3_5.ToMtemuFmt());
    }
    // 3.6
    {
        BinOp t_3_6(BinOp::Op::SUB, BinOpIn(rq, 3, r1));
        auto t_3_6_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1 });
        EXPECT_EQ(t_3_6_ans, t_3_6.ToMtemuFmt());
    }
    // 3.7
    {
        BinOp t_3_7(BinOp::Op::SUB, BinOpIn(rq, 15, rq));
        auto t_3_7_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 });
        EXPECT_EQ(t_3_7_ans, t_3_7.ToMtemuFmt());
    }
    // 3.8
    {
        BinOp t_3_8(BinOp::Op::SUB, BinOpIn(rq, rq, 17));
        auto t_3_8_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
        EXPECT_EQ(t_3_8_ans, t_3_8.ToMtemuFmt());
    }
    // 4.1
    {
        BinOp t_4_1(BinOp::Op::SUB, BinOpIn(r1, r1));
        auto t_4_1_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_4_1_ans, t_4_1.ToMtemuFmt());
    }
    // 4.2
    {
        BinOp t_4_2(BinOp::Op::SUB, BinOpIn(r1, r2));
        auto t_4_2_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_4_2_ans, t_4_2.ToMtemuFmt());
    }
    // 4.3
    {
        BinOp t_4_3(BinOp::Op::SUB, BinOpIn(r1, rq));
        auto t_4_3_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_4_3_ans, t_4_3.ToMtemuFmt());
    }
    // 4.4
    {
        BinOp t_4_4(BinOp::Op::SUB, BinOpIn(rq, r1));
        auto t_4_4_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_4_4_ans, t_4_4.ToMtemuFmt());
    }
    // 5.1
    {
        BinOp t_5_1(BinOp::Op::SUB, BinOpIn(r1, 128));
        auto t_5_1_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_5_1_ans, t_5_1.ToMtemuFmt());
    }
    // 5.2
    {
        BinOp t_5_2(BinOp::Op::SUB, BinOpIn(13, r1));
        auto t_5_2_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1 });
        EXPECT_EQ(t_5_2_ans, t_5_2.ToMtemuFmt());
    }
    // 5.3
    {
        BinOp t_5_3(BinOp::Op::SUB, BinOpIn(rq, 11));
        auto t_5_3_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1 });
        EXPECT_EQ(t_5_3_ans, t_5_3.ToMtemuFmt());
    }
    // 5.4
    {
        BinOp t_5_4(BinOp::Op::SUB, BinOpIn(6, rq));
        auto t_5_4_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0 });
        EXPECT_EQ(t_5_4_ans, t_5_4.ToMtemuFmt());
    }
}
