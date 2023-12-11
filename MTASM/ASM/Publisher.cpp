#include "Publisher.h"
#include "Exceptions.h"
#include <map>

Publisher::Publisher(std::ostream &out) : _cexprs(0), _out(out.rdbuf()) {}

void Publisher::Push(Expr &&expr)
{
    _qexpr.push_back(std::move(expr));
    ++_cexprs;
}

Address::Value Publisher::Push(const std::vector<Expr> &qexpr, const std::string &cmd)
{
    auto blockBegin = _cexprs;
    _addrTocmd.emplace(blockBegin, cmd);
    for (const auto &expr : qexpr)
    {
        if (expr->DependOnAddr())
            expr->IncrNextAddr(blockBegin);
        expr->SetCurAddr(_cexprs);
        _qexpr.push_back(expr);
        ++_cexprs;
    }
    return blockBegin;
}

Address::Value Publisher::GetAddr() const noexcept
{
    return _cexprs;
}

void Publisher::Write()
{
    auto size = static_cast<Value>(_qexpr.size());
    int shift = 0;
    for (Value i = 0; i < size; ++i)
    {
        if (_addrTocmd.contains(i) && _addrTocmd[i] == "MAIN")
        {
            shift = static_cast<int>(i);
            break;
        }
    }

    std::map<size_t, Expr> sortedExprs;
    std::unordered_map<Address::Value, std::string> realAddrToCmd;
    for (Value i = 0; i < size; ++i)
    {
        int real_cur_addr = _qexpr[i]->CurAddr().value();
        real_cur_addr -= shift;
        for (; real_cur_addr < 0; real_cur_addr += size);
        if (_qexpr[i]->DependOnAddr())
        {
            int real_next_addr = _qexpr[i]->NextAddr().value() - shift;
            for (; real_next_addr < 0; real_next_addr += size);
            _qexpr[i]->SetNextAddr(real_next_addr);
        }
        sortedExprs[real_cur_addr] = _qexpr[i];

        if (_addrTocmd.contains(i))
        {
            realAddrToCmd[real_cur_addr] = _addrTocmd[i];
            _addrTocmd.erase(i);
        }
    }

    for (const auto& [addr, expr] : sortedExprs)
    {
        if (realAddrToCmd.contains(addr))
            _out << ":" << realAddrToCmd[addr] << ":\n";
        _out << std::dec << addr << '\t' << std::dec << expr->NextAddr().value() << '\t' << std::dec << expr->ToMtemuFmt() << '\n';
    }
}

Publisher::~Publisher()
{
    Write();
}
