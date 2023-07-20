#include "Publisher.h"
#include "Exceptions.h"

constexpr const char *outName = "out.mtasm";

Publisher::Publisher() : _cexprs(0), _out(outName, std::ios::out | std::ios::binary)
{
    if (!_out)
        throw InternalCompilerError("Не удалось открыть файл для записи двоичных данных");
}

void Publisher::Push(std::unique_ptr<Expression> &&expr)
{
    _qexpr.push(std::move(expr));
    ++_cexprs;
}

void Publisher::Write()
{
    while (!_qexpr.empty())
    {
        _out << std::hex << _qexpr.front()->NextAddr().value() << ' ' << std::hex << _qexpr.front()->ToMtemuFmt() << '\n';
        _qexpr.pop();
    }
}

uint32_t Publisher::Size() const noexcept
{
    return _cexprs;
}

Publisher *Publisher::GetInstance()
{
    static Publisher publisher;
    return &publisher;
}

Publisher::~Publisher()
{
    Write();
    _out.close();
}
