#include <sstream>
#include <fstream>
#include <filesystem>
#include <gtest/gtest.h>
#include <ASM/ASM.h>
#include "MemLeakDetector.h"


TEST(Parser, _EmptyInput)
{
    MemoryLeakDetector mld;

    std::istringstream in;
    std::ostringstream out;
    yy::ASM mtasm(in, out);

    EXPECT_EQ(mtasm.Parse(), 0);
    EXPECT_TRUE(mtasm.GetEC().Empty());
    EXPECT_EQ(out.tellp(), 0);
}

TEST(Parser, _RegExceptions)
{
    MemoryLeakDetector mld;

    // 3 различных РОНа
    {
        std::istringstream in(R"(
                                    ADD R1, R0, R1;
                                    
                                    SUB R13, R14, R15;
                                    XOR R0, R1, R0;
                                )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_EQ(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 1);
        yy::location seLoc(yy::position(nullptr, 4, 1));
        EXPECT_EQ(se.front()._loc.value_or(yy::location()).begin.line, seLoc.begin.line);
        EXPECT_EQ(se.front()._msg, SE::BIN_3_DIFF_REG);
    }

    // 1 РОН в качестве левого и правого операнда
    {
        std::istringstream in(R"(
                                    NXOR R1, R11, R11;
                                    XOR RQ, RQ, R0;
                                )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_EQ(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 1);
        yy::location seLoc(yy::position(nullptr, 2, 1));
        EXPECT_EQ(se.front()._loc.value_or(yy::location()).begin.line, seLoc.begin.line);
        EXPECT_EQ(se.front()._msg, SE::BIN_1_L_R);
    }

    // RQ в качестве левого и правого операнда
    {
        std::istringstream in(R"(
                                    OR RQ, RQ;
                                    
                                    ADD 10, R1; 
                                )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_EQ(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 1);
        yy::location seLoc(yy::position(nullptr, 2, 1));
        EXPECT_EQ(se.front()._loc.value_or(yy::location()).begin.line, seLoc.begin.line);
        EXPECT_EQ(se.front()._msg, SE::BIN_Q_L_R);
    }

    // 3 ошибки
    {
        std::istringstream in(R"(
                                    AND R1, 10, R2;
                                    SUB RQ, RQ, RQ;

                                    ADD R0, R1, R2;


                                    NXOR    R1,         R0,     R0;
                                )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_EQ(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 3);
        EXPECT_EQ(se[0]._loc.value_or(yy::location()).begin.line, yy::location(yy::position(nullptr, 3, 1)).begin.line);
        EXPECT_EQ(se[0]._msg, SE::BIN_Q_L_R);
        EXPECT_EQ(se[1]._loc.value_or(yy::location()).begin.line, yy::location(yy::position(nullptr, 5, 1)).begin.line);
        EXPECT_EQ(se[1]._msg, SE::BIN_3_DIFF_REG);
        EXPECT_EQ(se[2]._loc.value_or(yy::location()).begin.line, yy::location(yy::position(nullptr, 8, 1)).begin.line);
        EXPECT_EQ(se[2]._msg, SE::BIN_1_L_R);
    }
}

// Все примеры из папки examples корректны
TEST(Parser, _Correct)
{
    MemoryLeakDetector mld;

    try
    {
        std::ostringstream out;
        const std::filesystem::path examples{ std::filesystem::current_path() / "examples" };
        for (const auto &example : std::filesystem::directory_iterator(examples))
        {
            if (!std::filesystem::is_regular_file(example.path()))
                continue;
            std::ifstream in(example.path(), std::ios::in | std::ios::binary);
            yy::ASM mtasm(in, out);
            EXPECT_EQ(mtasm.Parse(), 0);
            EXPECT_TRUE(mtasm.GetEC().Empty());
        }
    }
    catch (const std::exception &ex)
    {
        FAIL() << ex.what();
    }
}
