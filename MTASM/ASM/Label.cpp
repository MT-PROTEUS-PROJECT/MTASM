#include "Label.h"

size_t Label::Hash::operator()(const Label &lbl) const
{
    if (!lbl._cmd)
        return std::hash<std::string>()(lbl._lbl);
    return std::hash<std::string>()(*(lbl._cmd.get()) + ":" + lbl._lbl);
}

size_t Label::PtrHash::operator()(const std::shared_ptr<Label> &lbl) const
{
    return Label::Hash()(*lbl.get());
}

bool Label::PtrEqual::operator()(const std::shared_ptr<Label> &lhs, const std::shared_ptr<Label> &rhs) const
{
    return (*lhs.get()) == (*rhs.get());
}

Label::Label(const std::shared_ptr<std::string> &cmd, std::string lbl, Address addr) : _cmd(cmd), _lbl(std::move(lbl)), _addr(addr) {}

Label::Label(std::string lbl, Address addr) : _lbl(std::move(lbl)), _addr(addr) {}

std::string Label::GetStr() const noexcept
{
    return _lbl;
}

Address Label::GetAddr() const noexcept
{
    return _addr;
}

void Label::SetAddr(const Address &addr) noexcept
{
    _addr = addr;
}

void Label::IncrAddr(const Address &addr) noexcept
{
    _addr += addr;
}

bool operator==(const Label &lhs, const Label &rhs)
{
    return lhs._lbl == rhs._lbl;
}
