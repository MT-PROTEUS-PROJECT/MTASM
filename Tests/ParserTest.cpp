#include <sstream>
#include <fstream>
#include <filesystem>
#include <format>
#include <gtest/gtest.h>
#include <ASM/ASM.h>
#include "MemLeakDetector.h"

// В тестах намеренно используется кривое форматирование кода

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
                                    MAIN {
                                        ADD R1, R0, R1;
                                    
                                        SUB R13, R14, R15;
                                        XOR R0, R1, R0;
                                    }
                                )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_EQ(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 1);
        yy::location seLoc(yy::position(nullptr, 5, 1));
        EXPECT_EQ(se.front()._loc.value_or(yy::location()).begin.line, seLoc.begin.line);
        EXPECT_EQ(se.front()._msg, SE::BIN_3_DIFF_REG);
    }

    // 1 РОН в качестве левого и правого операнда
    {
        std::istringstream in(R"(MAIN {
                                    NXOR R1, R11, R11;
                                    XOR RQ, RQ, R0;
                                }
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
                                    MAIN
{
                                        OR RQ, RQ;
                                    
                                        ADD 10, R1; 
}
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
        EXPECT_EQ(se.front()._msg, SE::BIN_Q_L_R);
    }

    // 3 ошибки
    {
        std::istringstream in(R"(
MAIN
{
                                    AND R1, 10, R2;
                                    SUB RQ, RQ, RQ;

                                    ADD R0, R1, R2;


                                    NXOR    R1,         R0,     R0;
}
                                )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_EQ(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 3);
        EXPECT_EQ(se[0]._loc.value_or(yy::location()).begin.line, yy::location(yy::position(nullptr, 5, 1)).begin.line);
        EXPECT_EQ(se[0]._msg, SE::BIN_Q_L_R);
        EXPECT_EQ(se[1]._loc.value_or(yy::location()).begin.line, yy::location(yy::position(nullptr, 7, 1)).begin.line);
        EXPECT_EQ(se[1]._msg, SE::BIN_3_DIFF_REG);
        EXPECT_EQ(se[2]._loc.value_or(yy::location()).begin.line, yy::location(yy::position(nullptr, 10, 1)).begin.line);
        EXPECT_EQ(se[2]._msg, SE::BIN_1_L_R);
    }
}

TEST(Parser, _LabelExceptions)
{
    MemoryLeakDetector mld;

    // метка не найдена
    {
        std::istringstream in(R"(   MAIN
{
                                    AND R1, 10, R2;
                                    SUB RQ, R1, RQ;
                                    JZ L1;
                                    ADD R0, R0, R2;


                                    NXOR    R1,         R1,     R0;}
                                )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_EQ(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 1);
        EXPECT_EQ(se.front()._msg, std::format(SE::LBL_NOT_FOUND, "L1"));
    }

    // метка уже существует
    {
        std::istringstream in(R"(
MAIN{
                                    LabelName:
                                    AND R1, 10, R2;
                                    SUB RQ, R1, RQ;
                                    JZ LabelName;
                                    ADD R0, R0, R2;
                                    LabelName:

                                    NXOR    R1,         R1,     R0;
}
                                )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_EQ(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 1);
        EXPECT_EQ(se.front()._msg, std::format(SE::LBL_EXISTS, "LabelName"));
    }
}

TEST(Parser, _Cmd)
{
    MemoryLeakDetector mld;
    // команда не найдена
    {
        std::istringstream in(R"(   
                                F2 { SET R0, 1; RET; }
MAIN
{
                                AND R1, 10, R2;
                                SUB RQ, R1, RQ;
                                F2;
F1;
                                ADD R0, R0, R2;


                                NXOR    R1,         R1,     R0;}
                            )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_EQ(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 1);
        EXPECT_EQ(se.front()._loc.value_or(yy::location()).begin.line, yy::location(yy::position(nullptr, 8, 1)).begin.line);
        EXPECT_EQ(se.front()._msg, std::format(SE::CMD_NOT_FOUND, "F1"));
    }
    // команда не найдена
    {
        std::istringstream in(R"(   
                                F2 { SET R0, 1; F1; RET; }
                                F1 { ADD R0, 10; RET; }
MAIN
{
                                AND R1, 10, R2;
                                SUB RQ, R1, RQ;
                                F2;
F1;
                                ADD R0, R0, R2;


                                NXOR    R1,         R1,     R0;}
                            )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_EQ(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 1);
        EXPECT_EQ(se.front()._loc.value_or(yy::location()).begin.line, yy::location(yy::position(nullptr, 2, 1)).begin.line);
        EXPECT_EQ(se.front()._msg, std::format(SE::CMD_NOT_FOUND, "F1"));
    }
    // команда с таким названием уже существует
    {
        std::istringstream in(R"(   
                                F2 { SET R0, 1; RET; }
                                F1 { ADD R3, R3, R2; RET; }
                                F2 { SET R2, 1; RET; }
MAIN
{
                                AND R1, 10, R2;
                                SUB RQ, R1, RQ;
                                F2;
F1;
                                ADD R0, R0, R2;


                                NXOR    R1,         R1,     R0;}
                            )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_EQ(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 1);
        EXPECT_EQ(se.front()._loc.value_or(yy::location()).begin.line, yy::location(yy::position(nullptr, 4, 1)).begin.line);
        EXPECT_EQ(se.front()._msg, std::format(SE::CMD_EXISTS, "F2"));
    }
    // команда с таким названием уже существует
    {
        std::istringstream in(R"(
MAIN
{
                                AND R1, 10, R2;
                                SUB RQ, R1, RQ;
                                ADD R0, R0, R2;


                                NXOR    R1,         R1,     R0;}
MAIN { RET; }
                            )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_GT(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 1);
        EXPECT_EQ(se.front()._loc.value_or(yy::location()).begin.line, yy::location(yy::position(nullptr, 10, 1)).begin.line);
    }
    // команды с пустым телом не поддерживаются
    {
        std::istringstream in(R"(
    _CMD {}
MAIN
{
                                AND R1, 10, R2;
                                SUB RQ, R1, RQ;
                                ADD R0, R0, R2;
                                _CMD;

                                NXOR    R1,         R1,     R0;}
                            )");
        std::ostringstream out;
        yy::ASM mtasm(in, out);

        EXPECT_GT(mtasm.Parse(), 0);
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::ICE).empty());
        EXPECT_TRUE(mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER).empty());
        auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
        EXPECT_EQ(se.size(), 1);
        EXPECT_EQ(se.back()._loc.value_or(yy::location()).begin.line, yy::location(yy::position(nullptr, 2, 1)).begin.line);
    }
}

// Все примеры из папки examples корректны
TEST(Parser, _Correct)
{
    MemoryLeakDetector mld;

    try
    {
        const std::filesystem::path examples{ std::filesystem::current_path() / "examples" };
        for (const auto &example : std::filesystem::directory_iterator(examples))
        {
            if (!std::filesystem::is_regular_file(example.path()))
                continue;
#ifdef WRITE_RESULT
            std::ofstream out(examples / (example.path().stem().string() + ".mtasm"), std::ios::out | std::ios::binary);
#else
            std::ostringstream out;
#endif
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
