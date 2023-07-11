#include "Input.h"
#include "Exceptions.h"

ArOpIn::ArOpIn(Register r1, Register r2, Register r3) : _load(true), _hasRQ(r2.isRQ() || r3.isRQ()), _nullPos(-1), _value(0)
{
    _regs.push_back(std::make_unique<Register>(std::move(r1)));
    _regs.push_back(std::make_unique<Register>(std::move(r2)));
    _regs.push_back(std::make_unique<Register>(std::move(r3)));
}

ArOpIn::ArOpIn(Register r1, Register r2, Value v) : _load(true), _hasRQ(r2.isRQ()), _nullPos(2), _value(v)
{
    _regs.push_back(std::make_unique<Register>(std::move(r1)));
    _regs.push_back(std::make_unique<Register>(std::move(r2)));
    _regs.push_back(nullptr);
}

ArOpIn::ArOpIn(Register r1, Value v, Register r2) : _load(true), _hasRQ(r2.isRQ()), _nullPos(1), _value(v)
{
    _regs.push_back(std::make_unique<Register>(std::move(r1)));
    _regs.push_back(nullptr);
    _regs.push_back(std::make_unique<Register>(std::move(r2)));
}

ArOpIn::ArOpIn(Register r1, Register r2) : _load(false), _hasRQ(r1.isRQ() || r2.isRQ()), _nullPos(-1), _value(0)
{
    _regs.push_back(nullptr);
    _regs.push_back(std::make_unique<Register>(std::move(r1)));
    _regs.push_back(std::make_unique<Register>(std::move(r2)));
}

ArOpIn::ArOpIn(Register r1, Value v) : _load(false), _hasRQ(r1.isRQ()), _nullPos(2), _value(v)
{
    _regs.push_back(nullptr);
    _regs.push_back(std::make_unique<Register>(std::move(r1)));
    _regs.push_back(nullptr);
}

ArOpIn::ArOpIn(Value v, Register r1) : _load(false), _hasRQ(r1.isRQ()), _nullPos(1), _value(v)
{
    _regs.push_back(nullptr);
    _regs.push_back(nullptr);
    _regs.push_back(std::make_unique<Register>(std::move(r1)));
}

uint32_t ArOpIn::ToMtemuFmt() const
{
    //         M1 I6-I8                             M0 I0-I2                 C0 I3-I5                  A                  B            D
    //   8388608  4194304 2097152 1048576  524286 262144 131072 65536   32768 16384 8192 4096   2048 1024 512 256   128 64 32 16    8 4 2 1    
    //      0   '    0   '   0   '   0        0  '   0  '   0  '  0        0 '  0  ' 0 '  0       0 '  0 ' 0 ' 0     0 ' 0' 0' 0    0'0'0'0
    uint32_t bits = 0;

    // Загрузка (I6-I8)
    // 000 - Загрузка в PQ
    // 001 - Нет загрузки
    // 011 - Загрузка в РОН(В)
    if (_load)
    {
        if (!_regs[0]->isRQ())
            bits += 3;
    }
    else
    {
        bits += 1;
    }
    bits <<= 4;

    // Выбор операндов R и S (I0-I2)
    int8_t ops = -1;

    if (!_hasRQ && _nullPos == -1) // РОН(А) РОН(В)
    {
        bits += 1;
        ops = 1;
    }
    else if (!_hasRQ && _nullPos != -1) // D РОН(А)
    {
        bits += 5;
        ops = 2;
    }
    else if (_hasRQ && _nullPos == -1) // РОН(А) PQ
    {
        ops = 0;
    }
    else if (_hasRQ && _nullPos != -1) // D PQ
    {
        bits += 6;
        ops = 3;
    }
    else
    {
        throw InternalCompilerError("Не удалось определить операнды R и S (тетрада I0-I2)");
    }
    bits <<= 4;

    // Тип операции I3-I5 (заполняется в другом классе)
    bits <<= WORD_SIZE;
    
    // Оставляем младшие WORD_SIZE бита у _value
    _value <<= (sizeof(_value) * 8) - WORD_SIZE;
    _value >>= (sizeof(_value) * 8) - WORD_SIZE;

    // А B D
    if (ops == 0)
    {
        if (!_regs[1]->isRQ())
        {
            bits += _regs[1]->addr().value();
            bits <<= WORD_SIZE;
        }
        else
        {
            bits += _regs[2]->addr().value();
            bits <<= WORD_SIZE;
        }
        if (_load && !_regs[0]->isRQ())
        {
            bits += _regs[0]->addr().value();
        }
        bits <<= WORD_SIZE;
    }
    else if (ops == 1)
    {
        if (!_load)
        {
            bits += _regs[1]->addr().value();
            bits <<= WORD_SIZE;

            bits += _regs[2]->addr().value();
            bits <<= WORD_SIZE;
        }
        else
        {
            if ((*_regs[0].get()) == (*_regs[1].get()))
            {
                bits += _regs[2]->addr().value();
                bits <<= WORD_SIZE;

                bits += _regs[1]->addr().value();
                bits <<= WORD_SIZE;
            }
            else
            {
                bits += _regs[1]->addr().value();
                bits <<= WORD_SIZE;

                bits += _regs[2]->addr().value();
                bits <<= WORD_SIZE;
            }
        }
    }
    else if (ops == 2)
    {
        if (_nullPos == 1)
        {
            bits += _regs[2]->addr().value();
            bits <<= WORD_SIZE;
        }
        else
        {
            bits += _regs[1]->addr().value();
            bits <<= WORD_SIZE;
        }
        if (_load && !_regs[0]->isRQ())
        {
            bits += _regs[0]->addr().value();
        }
        bits <<= WORD_SIZE;
        bits += _value;
    }
    else
    {
        bits <<= WORD_SIZE;
        if (_load && !_regs[0]->isRQ())
        {
            bits += _regs[0]->addr().value();
        }
        bits <<= WORD_SIZE;
        bits += _value;
    }
    return bits;
}
