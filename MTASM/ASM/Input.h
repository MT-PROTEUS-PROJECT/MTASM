#pragma once

#include "Register.h"
#include "TypeDefs.h"

#include <vector>
#include <bitset>
#include <memory>

class Input
{
protected:
    Value _mtemuFmt = 0;

protected:
    virtual void calcMtemu(bool force) = 0;

public:
    Input() = default;

    Value ToMtemuFmt() const;

    virtual ~Input() = default;
};

class BinOpIn final : public Input
{
public:
    using RegContainer = std::vector<std::shared_ptr<Register>>;

private:
    bool _load;
    bool _hasRQ;
    int _nullPos;
    Value _value;
    RegContainer _regs;
    RegContainer _template_regs;

    void calcMtemu(bool force = false) override;
public:
    BinOpIn(Register r1, Register r2, Register r3);
    BinOpIn(Register r1, Register r2, Value v);
    BinOpIn(Register r1, Value v, Register r2);
    BinOpIn(Register r1, Register r2);
    BinOpIn(Register r1, Value v);
    BinOpIn(Value v, Register r1);

    int GetNullPos() const noexcept;
    const RegContainer& GetRegs() const noexcept;
    RegContainer& GetRegs() noexcept;
    const RegContainer& GetTemplateRegs() const noexcept;
    void Update(const RegContainer &regs);

    ~BinOpIn() = default;
};
