#include "Label.h"

size_t Label::Hash::operator()(const Label &lbl) const
{
    return std::hash<std::string>()(lbl._lbl);
}

Label::Label(std::string lbl, uint32_t addr) : _lbl(std::move(lbl)), _addr(addr) {}

std::string Label::GetStr() const noexcept
{
    return _lbl;
}

uint32_t Label::GetAddr() const noexcept
{
    return _addr;
}

void Label::SetAddr(uint32_t addr) noexcept
{
    _addr = addr;
}

bool operator==(const Label &lhs, const Label &rhs)
{
    return lhs._lbl == rhs._lbl;
}
