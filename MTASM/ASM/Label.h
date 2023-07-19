#pragma once

#include <string>
#include <memory>

#include "Address.h"

class Label final
{
private:
    std::string _lbl;
    Address _addr;

public:
    struct Hash
    {
        size_t operator()(const Label &lbl) const;
    };

    struct PtrHash
    {
        size_t operator()(const std::shared_ptr<Label> &lbl) const;
    };

    struct PtrEqual
    {
        bool operator()(const std::shared_ptr<Label> &lhs, const std::shared_ptr<Label> &rhs) const;
    };

    friend bool operator==(const Label &lhs, const Label &rhs);

public:
    Label(std::string lbl, Address addr = {});

    Label(const Label &) = default;
    Label &operator=(const Label &) = default;
    Label(Label &&) = default;
    Label &operator=(Label &&) = default;

    ~Label() = default;

public:
    std::string GetStr() const noexcept;

    Address GetAddr() const noexcept;
    void SetAddr(const Address &addr) noexcept;
    void IncrAddr(const Address &addr) noexcept;
};
