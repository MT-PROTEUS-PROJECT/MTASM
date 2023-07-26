#include "Publisher.h"
#include "Exceptions.h"

Publisher::Publisher(std::ostream &out) : _cexprs(0), _out(out.rdbuf()) {}

void Publisher::Push(Expr &&expr)
{
    _qexpr.push_back(std::move(expr));
    ++_cexprs;
}

void Publisher::Push(std::vector<Expr> &qexpr)
{
    auto blockBegin = _cexprs;
    for (auto &&expr : qexpr)
    {
        expr->IncrAddr(blockBegin);
        _qexpr.push_back(std::move(expr));
        ++_cexprs;
    }
}

void Publisher::Write()
{
    for (const auto &expr : _qexpr)
    {
        _out << std::hex << expr->NextAddr().value() << '\t' << std::hex << expr->ToMtemuFmt() << '\n';
    }
}

uint32_t Publisher::Size() const noexcept
{
    return _cexprs;
}

Publisher::~Publisher()
{
    Write();
    _qexpr.clear();
}
