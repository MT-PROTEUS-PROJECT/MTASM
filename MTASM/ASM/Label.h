#pragma once

#include <string>

class Label final
{
private:
    std::string _lbl;
    uint32_t _addr;

public:
    struct Hash
    {
        size_t operator()(const Label &lbl) const;
    };

    friend bool operator==(const Label &lhs, const Label &rhs);

public:
    Label(std::string lbl, uint32_t addr = 1u);

    Label(const Label &) = default;
    Label &operator=(const Label &) = default;
    Label(Label &&) = default;
    Label &operator=(Label &&) = default;

    std::string GetStr() const noexcept;
    uint32_t GetAddr() const noexcept;
    void SetAddr(uint32_t addr) noexcept;

    ~Label() = default;
};
