#include "Parser.tab.hh"
#include <Lexer/Lexer.h>
#include <ASM/Exceptions.h>

#include <fstream>

#ifdef _DEBUG
constexpr const char *file = "TestFile.txt";
#endif

int main(int argc, char **argv)
{
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
        throw std::runtime_error("Неверное число аргументов!\n Usage: MTASM.exe path_to_file.txt");
#endif
    }

    if (!in.is_open())
        throw std::runtime_error("Не удалось открыть файл для чтения");

    yy::Lexer lexer(in);
    yy::parser parser{ lexer };
    try
    {
        int err = parser();
        if (err)
            std::cout << "Parse error";
    }
    catch (const InternalCompilerError &ice)
    {
        std::cerr << "Внутренняя ошибка компилятора: " << ice.what() << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    return 0;
}
