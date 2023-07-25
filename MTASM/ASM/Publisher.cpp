#include "Publisher.h"
#include "Exceptions.h"

constexpr const char *outName = "out.mtasm";

Publisher::Publisher() : _cexprs(0), _out(outName, std::ios::out | std::ios::binary)
{
    if (!_out)
        throw InternalCompilerError("Не удалось открыть файл для записи двоичных данных");
}

void Publisher::Push(Expr &&expr)
{
    _qexpr.push(std::move(expr));
    ++_cexprs;
}

void Publisher::Push(std::queue<Expr> &qexpr)
{
    auto blockBegin = _cexprs;
    while (!qexpr.empty())
    {
        auto expr = std::move(qexpr.front());
        expr->IncrAddr(blockBegin);
        _qexpr.push(std::move(expr));
        ++_cexprs;
        qexpr.pop();
    }
}

void Publisher::Write()
{
    while (!_qexpr.empty())
    {
        _out << std::hex << _qexpr.front()->NextAddr().value() << '\t' << std::hex << _qexpr.front()->ToMtemuFmt() << '\n';
        _qexpr.pop();
    }
}

uint32_t Publisher::Size() const noexcept
{
    return _cexprs;
}

Publisher::~Publisher()
{
    Write();
    _out.close();
}
