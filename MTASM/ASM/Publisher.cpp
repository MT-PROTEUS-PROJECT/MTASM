#include "Publisher.h"
#include "Exceptions.h"

Publisher::Publisher(std::ostream &out) : _cexprs(0), _out(out.rdbuf()) {}

void Publisher::Push(Expr &&expr)
{
    _qexpr.push_back(std::move(expr));
    ++_cexprs;
}

Address::Value Publisher::Push(std::vector<Expr> &qexpr, const std::string &cmd)
{
    auto blockBegin = _cexprs;
    _addrTocmd.emplace(blockBegin, cmd);
    for (auto &&expr : qexpr)
    {
        expr->IncrAddr(blockBegin);
        _qexpr.push_back(std::move(expr));
        ++_cexprs;
    }
    return blockBegin;
}

void Publisher::Write()
{
    auto size = static_cast<Value>(_qexpr.size());
    for (Value i = 0; i < size; ++i)
    {
        if (_addrTocmd.contains(i))
            _out << ":" << _addrTocmd[i] << ":\n";
        _out << std::hex << _qexpr[i]->NextAddr().value() << '\t' << std::hex << _qexpr[i]->ToMtemuFmt() << '\n';
    }
}

Publisher::~Publisher()
{
    Write();
    _qexpr.clear();
    _addrTocmd.clear();
}
