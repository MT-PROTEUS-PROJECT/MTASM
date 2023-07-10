#pragma once

#include "Register.h"
#include "TypeDefs.h"

#include <vector>
#include <memory>

class Input
{
public:
    Input() = default;

    virtual ~Input() = default;
};

class ArOpIn final: public Input
{
private:
    bool _load;
    Value _value;
    std::vector<std::unique_ptr<Register>> _regs;
    
public:
    ArOpIn(Register r1, Register r2, Register r3);
    ArOpIn(Register r1, Register r2, Value v);
    ArOpIn(Register r1, Value v, Register r2);
    ArOpIn(Register r1, Register r2);
    ArOpIn(Register r1, Value v);
    ArOpIn(Value v, Register r1);

    ~ArOpIn() = default;
};
