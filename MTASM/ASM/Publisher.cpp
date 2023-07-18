#include "Publisher.h"

#include "Exceptions.h"

constexpr char delim = '\n';
constexpr size_t maxQSize = 1024;

constexpr const char *outName = "out.mtasm";

Publisher::Publisher() : _out(outName, std::ios::out | std::ios::binary)
{
    if (!_out)
        throw InternalCompilerError("Не удалось открыть файл для записи двоичных данных");
}

void Publisher::Push(const std::shared_ptr<Expression> &expr)
{
    _qexpr.push(expr);
    if (_qexpr.size() >= maxQSize)
        Write();
}

void Publisher::Write()
{
    while (!_qexpr.empty())
    {
        _out << std::hex << _qexpr.front()->ToMtemuFmt() << delim;
        _qexpr.pop();
    }
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
