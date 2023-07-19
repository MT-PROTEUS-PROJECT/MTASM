#include "Parser.tab.hh"
#include <Lexer/Lexer.h>
#include <ASM/Exceptions.h>
#include <Utils/Logger.h>

#include <fstream>

#ifdef _DEBUG
constexpr const char *file = "TestFile.txt";
#endif

#include <Windows.h>

int main(int argc, char **argv)
{
#ifndef _DEBUG
    ShowWindow(GetConsoleWindow(), SW_HIDE);
#else
    ShowWindow(GetConsoleWindow(), SW_SHOW);
#endif
    setlocale(LC_ALL, "Russian");

    std::ifstream in;
    if (argc == 2)
    {
        in.open(argv[1], std::ios::in);
    }
    else
    {
#ifdef _DEBUG
        in.open(file, std::ios::in);
#else
        throw std::runtime_error("�������� ����� ����������!\n Usage: MTASM.exe path_to_file.txt");
#endif
    }

    if (!in.is_open())
        throw std::runtime_error("�� ������� ������� ���� ��� ������");

    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);
    yy::Lexer lexer(in);
    yy::parser parser{ lexer };
//#ifdef _DEBUG
//    parser.set_debug_level(1);
//#endif
    try
    {
        int err = parser();
        if (err)
            std::cout << "Parse error";
    }
    catch (const InternalCompilerError &ice)
    {
        std::cerr << "���������� ������ �����������: " << ice.what() << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    return 0;
}
