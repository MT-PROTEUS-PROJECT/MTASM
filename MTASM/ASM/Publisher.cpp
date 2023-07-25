#include "Publisher.h"
#include "Exceptions.h"

Publisher::Publisher(std::ostream &out) : _cexprs(0), _out(out.rdbuf()) {}

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
}
