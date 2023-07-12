#pragma once

#include "Register.h"
#include "TypeDefs.h"

#include <vector>
#include <bitset>
#include <memory>

class Input
{
protected:
    uint32_t _mtemuFmt = 0;

protected:
    virtual void calcMtemu() = 0;

public:
    Input() = default;
    
    uint32_t ToMtemuFmt() const;
    
    virtual ~Input() = default;
};

class ArOpIn final: public Input
{
public:
    using RegContainer = std::vector<std::shared_ptr<Register>>;

private:
    bool _load;
    bool _hasRQ;
    int _nullPos;
    Value _value;
    RegContainer _regs;
    
    void calcMtemu() override;
public:
    ArOpIn(Register r1, Register r2, Register r3);
    ArOpIn(Register r1, Register r2, Value v);
    ArOpIn(Register r1, Value v, Register r2);
    ArOpIn(Register r1, Register r2);
    ArOpIn(Register r1, Value v);
    ArOpIn(Value v, Register r1);

    int GetNullPos() const noexcept;
    const RegContainer &GetRegs() const noexcept;

    ~ArOpIn() = default;
};
