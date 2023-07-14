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
// 1.1  | ADD RQ, R2, 30 | 0010  0   000  0   101  0   000  0010 0000 1110 |
// 1.2  | ADD R1, R10, RQ| 0010  0   011  0   000  0   000  1010 0001 0000 |
// 1.3  | ADD R3, R4     | 0010  0   001  0   001  0   000  0011 0100 0000 |
// 1.4  | ADD 10, R6     | 0010  0   001  0   101  0   000  0110 0000 1010 |
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
//                             LOGICAL
// -----|----------------|-------------------------------------------------|
// 6.1  | OR R1, 12, RQ  | 0010  0   011  0   110  0   011  0000 0001 1100 |
// 6.2  | OR RQ, RQ, R11 | 0010  0   000  0   000  0   011  1011 0000 0000 |
// 6.3  |   OR R5, R6    | 0010  0   001  0   001  0   011  0101 0110 0000 |
// 6.4  |   OR R13, 10   | 0010  0   001  0   101  0   011  1101 0000 1010 |
// -----|----------------|-------------------------------------------------|
// 7.1  | AND R7, RQ, 8  | 0010  0   011  0   110  0   100  0000 0111 1000 |
// 7.2  | AND RQ, R8, RQ | 0010  0   000  0   000  0   100  1000 0000 0000 |
// 7.3  |   AND R9, R10  | 0010  0   001  0   001  0   100  1001 1010 0000 |
// 7.4  |   AND 12, R11  | 0010  0   001  0   101  0   100  1011 0000 1100 |
// -----|----------------|-------------------------------------------------|
// 8.1  | XOR R12, 9, RQ | 0010  0   011  0   110  0   110  0000 1100 1001 |
// 8.2  | XOR RQ, RQ, R13| 0010  0   000  0   000  0   110  1101 0000 0000 |
// 8.3  |   XOR R14, R15 | 0010  0   001  0   001  0   110  1110 1111 0000 |
// 8.4  |   XOR R0, 11   | 0010  0   001  0   101  0   110  0000 0000 1011 |
// -----|----------------|-------------------------------------------------|
// 9.1  |NXOR R13, RQ, 10| 0010  0   011  0   110  0   111  0000 1101 1010 |
// 9.2  |NXOR RQ, R14, RQ| 0010  0   000  0   000  0   111  1110 0000 0000 |
// 9.3  |  NXOR R15, R0  | 0010  0   001  0   001  0   111  1111 0000 0000 |
// 9.4  |  NXOR R1, 14   | 0010  0   001  0   101  0   111  0001 0000 1110 |
// -----|----------------|-------------------------------------------------|

// Test 1
TEST(BinOp, _Add)
{
    MemoryLeakDetector mld;
    
    Register r2("R2");
    Register rq("RQ");
    // 1.1
    {
        BinOp t_1_1(BinOp::Op::ADD, BinOpIn(rq, r2, 30));
        auto t_1_1_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0 });
        EXPECT_EQ(t_1_1_ans, t_1_1.ToMtemuFmt());
    }
    // 1.2
    {
        BinOp t_1_2(BinOp::Op::ADD, BinOpIn(Register("R1"), Register("R10"), rq));
        auto t_1_2_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_1_2_ans, t_1_2.ToMtemuFmt());
    }
    // 1.3
    {
        BinOp t_1_3(BinOp::Op::ADD, BinOpIn(Register("R3"), Register("R4")));
        auto t_1_3_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_1_3_ans, t_1_3.ToMtemuFmt());
    }
    // 1.4
    {
        BinOp t_1_4(BinOp::Op::ADD, BinOpIn(10, Register("R6")));
        auto t_1_4_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0 });
        EXPECT_EQ(t_1_4_ans, t_1_4.ToMtemuFmt());
    }
}

// Test 2-5
TEST(BinOp, _Sub)
{
    MemoryLeakDetector mld;

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

// Test 6
TEST(BinOp, _Or)
{
    MemoryLeakDetector mld;

    Register r1("R1");
    Register rq("RQ");

    // 6.1
    {
        BinOp t_6_1(BinOp::Op::OR, BinOpIn(r1, 12, rq));
        auto t_6_1_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0 });
        EXPECT_EQ(t_6_1_ans, t_6_1.ToMtemuFmt());
    }
    // 6.2
    {
        BinOp t_6_2(BinOp::Op::OR, BinOpIn(rq, rq, Register("R11")));
        auto t_6_2_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_6_2_ans, t_6_2.ToMtemuFmt());
    }
    // 6.3
    {
        BinOp t_6_3(BinOp::Op::OR, BinOpIn(Register("R5"), Register("R6")));
        auto t_6_3_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_6_3_ans, t_6_3.ToMtemuFmt());
    }
    // 6.4
    {
        BinOp t_6_4(BinOp::Op::OR, BinOpIn(Register("R13"), 10));
        auto t_6_4_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0 });
        EXPECT_EQ(t_6_4_ans, t_6_4.ToMtemuFmt());
    }
}

// Test 7
TEST(BinOp, _And)
{
    MemoryLeakDetector mld;

    // 7.1
    {
        BinOp t_7_1(BinOp::Op::AND, BinOpIn(Register("R7"), Register("RQ"), 8));
        auto t_7_1_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0 });
        EXPECT_EQ(t_7_1_ans, t_7_1.ToMtemuFmt());
    }
    // 7.2
    {
        BinOp t_7_2(BinOp::Op::AND, BinOpIn(Register("RQ"), Register("R8"), Register("RQ")));
        auto t_7_2_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_7_2_ans, t_7_2.ToMtemuFmt());
    }
    // 7.3
    {
        BinOp t_7_3(BinOp::Op::AND, BinOpIn(Register("R9"), Register("R10")));
        auto t_7_3_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_7_3_ans, t_7_3.ToMtemuFmt());
    }
    // 7.4
    {
        BinOp t_7_4(BinOp::Op::AND, BinOpIn(12, Register("R11")));
        auto t_7_4_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0 });
        EXPECT_EQ(t_7_4_ans, t_7_4.ToMtemuFmt());
    }
}

// Test 8
TEST(BinOp, _Xor)
{
    MemoryLeakDetector mld;

    // 8.1
    {
        BinOp t_8_1(BinOp::Op::XOR, BinOpIn(Register("R12"), 9, Register("RQ")));
        auto t_8_1_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1 });
        EXPECT_EQ(t_8_1_ans, t_8_1.ToMtemuFmt());
    }
    // 8.2
    {
        BinOp t_8_2(BinOp::Op::XOR, BinOpIn(Register("RQ"), Register("RQ"), Register("R13")));
        auto t_8_2_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_8_2_ans, t_8_2.ToMtemuFmt());
    }
    // 8.3
    {
        BinOp t_8_3(BinOp::Op::XOR, BinOpIn(Register("R14"), Register("R15")));
        auto t_8_3_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0 });
        EXPECT_EQ(t_8_3_ans, t_8_3.ToMtemuFmt());
    }
    // 8.4
    {
        BinOp t_8_4(BinOp::Op::XOR, BinOpIn(Register("R0"), 11));
        auto t_8_4_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1 });
        EXPECT_EQ(t_8_4_ans, t_8_4.ToMtemuFmt());
    }
}

// Test 9
TEST(BinOp, _Nxor)
{
    MemoryLeakDetector mld;

    // 9.1
    {
        BinOp t_9_1(BinOp::Op::NXOR, BinOpIn(Register("R13"), Register("RQ"), 10));
        auto t_9_1_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0 });
        EXPECT_EQ(t_9_1_ans, t_9_1.ToMtemuFmt());
    }
    // 9.2
    {
        BinOp t_9_2(BinOp::Op::NXOR, BinOpIn(Register("RQ"), Register("R14"), Register("RQ")));
        auto t_9_2_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_9_2_ans, t_9_2.ToMtemuFmt());
    }
    // 9.3
    {
        BinOp t_9_3(BinOp::Op::NXOR, BinOpIn(Register("R15"), Register("R0")));
        auto t_9_3_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
        EXPECT_EQ(t_9_3_ans, t_9_3.ToMtemuFmt());
    }
    // 9.4
    {
        BinOp t_9_4(BinOp::Op::NXOR, BinOpIn(Register("R1"), 14));
        auto t_9_4_ans = ToMtemuFmt({ 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0 });
        EXPECT_EQ(t_9_4_ans, t_9_4.ToMtemuFmt());
    }
}
