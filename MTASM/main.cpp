#include "Parser.tab.hh"
#include <Lexer/Lexer.h>
#include <ASM/Exceptions.h>

int main()
{
    setlocale(LC_ALL, "Russian");
    yy::Lexer lexer(std::cin);
    yy::parser parser{ lexer };
    try
    {
        int ok = parser();
        if (!ok)
            std::cout << "Parse error";
    }
    catch (const InternalCompilerError &ice)
    {
        std::cerr << "Внутрення ошибка компилятора: " << ice.what() << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    return 0;
}
