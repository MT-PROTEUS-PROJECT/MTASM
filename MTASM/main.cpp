#include "Parser.tab.hh"
#include <Lexer/Lexer.h>

int main()
{
    yy::Lexer lexer(std::cin);
    yy::parser parser{ lexer };
    int ok = parser();
    if (!ok)
        std::cout << "Parse error";
    return 0;
}
