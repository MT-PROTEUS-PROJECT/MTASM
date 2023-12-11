#include "Input.h"
#include "Exceptions.h"

Value Input::ToMtemuFmt() const
{
    return _mtemuFmt;
}

BinOpIn::BinOpIn(Register r1, Register r2, Register r3) : _load(true), _hasRQ(r2.isRQ() || r3.isRQ()), _nullPos(-1), _value(0)
{
    _regs.push_back(std::make_shared<Register>(std::move(r1)));
    _regs.push_back(std::make_shared<Register>(std::move(r2)));
    _regs.push_back(std::make_shared<Register>(std::move(r3)));
    calcMtemu();
}

BinOpIn::BinOpIn(Register r1, Register r2, Value v) : _load(true), _hasRQ(r2.isRQ()), _nullPos(2), _value(v)
{
    _regs.push_back(std::make_shared<Register>(std::move(r1)));
    _regs.push_back(std::make_shared<Register>(std::move(r2)));
    _regs.push_back(nullptr);
    calcMtemu();
}

BinOpIn::BinOpIn(Register r1, Value v, Register r2) : _load(true), _hasRQ(r2.isRQ()), _nullPos(1), _value(v)
{
    _regs.push_back(std::make_shared<Register>(std::move(r1)));
    _regs.push_back(nullptr);
    _regs.push_back(std::make_shared<Register>(std::move(r2)));
    calcMtemu();
}

BinOpIn::BinOpIn(Register r1, Register r2) : _load(false), _hasRQ(r1.isRQ() || r2.isRQ()), _nullPos(-1), _value(0)
{
    _regs.push_back(nullptr);
    _regs.push_back(std::make_shared<Register>(std::move(r1)));
    _regs.push_back(std::make_shared<Register>(std::move(r2)));
    calcMtemu();
}

BinOpIn::BinOpIn(Register r1, Value v) : _load(false), _hasRQ(r1.isRQ()), _nullPos(2), _value(v)
{
    _regs.push_back(nullptr);
    _regs.push_back(std::make_shared<Register>(std::move(r1)));
    _regs.push_back(nullptr);
    calcMtemu();
}

BinOpIn::BinOpIn(Value v, Register r1) : _load(false), _hasRQ(r1.isRQ()), _nullPos(1), _value(v)
{
    _regs.push_back(nullptr);
    _regs.push_back(nullptr);
    _regs.push_back(std::make_shared<Register>(std::move(r1)));
    calcMtemu();
}

void BinOpIn::calcMtemu(bool force)
{
    //         M1 I6-I8                             M0 I0-I2                 C0 I3-I5                  A                  B            D
    //   8388608  4194304 2097152 1048576  524286 262144 131072 65536   32768 16384 8192 4096   2048 1024 512 256   128 64 32 16    8 4 2 1    
    //      0   '    0   '   0   '   0        0  '   0  '   0  '  0        0 '  0  ' 0 '  0       0 '  0 ' 0 ' 0     0 ' 0' 0' 0    0'0'0'0
    _mtemuFmt = 0;

    if (!force)
    {
        for (const auto& reg : _regs)
        {
            if (reg && reg->isTemplate())
            {
                _template_regs.push_back(reg);
            }
        }

        if (!_template_regs.empty())
            return;
    }
    // Загрузка (I6-I8)
    // 000 - Загрузка в PQ
    // 001 - Нет загрузки
    // 011 - Загрузка в РОН(В)
    if (_load)
    {
        if (!_regs[0]->isRQ())
            _mtemuFmt += 3;
    }
    else
    {
        _mtemuFmt += 1;
    }
    _mtemuFmt <<= 4;

    // Выбор операндов R и S (I0-I2)
    int8_t ops = -1;

    if (!_hasRQ && _nullPos == -1) // РОН(А) РОН(В)
    {
        _mtemuFmt += 1;
        ops = 1;
    }
    else if (!_hasRQ && _nullPos != -1) // D РОН(А)
    {
        _mtemuFmt += 5;
        ops = 2;
    }
    else if (_hasRQ && _nullPos == -1) // РОН(А) PQ
    {
        ops = 0;
    }
    else if (_hasRQ && _nullPos != -1) // D PQ
    {
        _mtemuFmt += 6;
        ops = 3;
    }
    else
    {
        throw InternalCompilerError("Не удалось определить операнды R и S (тетрада I0-I2)");
    }
    _mtemuFmt <<= 4;

    // Тип операции I3-I5 (заполняется в другом классе)
    _mtemuFmt <<= 4;

    // Оставляем младшие WORD_SIZE бита у _value
    _value <<= (sizeof(_value) * 8) - WORD_SIZE;
    _value >>= (sizeof(_value) * 8) - WORD_SIZE;

    // А B D
    if (ops == 0)
    {
        if (!_regs[1]->isRQ())
        {
            _mtemuFmt += _regs[1]->addr().value();
        }
        else
        {
            _mtemuFmt += _regs[2]->addr().value();
        }
        _mtemuFmt <<= ADDR_SIZE;

        if (_load && !_regs[0]->isRQ())
        {
            _mtemuFmt += _regs[0]->addr().value();
        }
        _mtemuFmt <<= WORD_SIZE;
    }
    else if (ops == 1)
    {
        if (!_load)
        {
            _mtemuFmt += _regs[1]->addr().value();
            _mtemuFmt <<= ADDR_SIZE;

            _mtemuFmt += _regs[2]->addr().value();
            _mtemuFmt <<= WORD_SIZE;
        }
        else
        {
            if ((*_regs[0].get()) == (*_regs[1].get()))
            {
                _mtemuFmt += _regs[2]->addr().value();
                _mtemuFmt <<= ADDR_SIZE;

                _mtemuFmt += _regs[1]->addr().value();
                _mtemuFmt <<= WORD_SIZE;
            }
            else
            {
                _mtemuFmt += _regs[1]->addr().value();
                _mtemuFmt <<= ADDR_SIZE;

                _mtemuFmt += _regs[2]->addr().value();
                _mtemuFmt <<= WORD_SIZE;
            }
        }
    }
    else if (ops == 2)
    {
        if (_nullPos == 1)
        {
            _mtemuFmt += _regs[2]->addr().value();
        }
        else
        {
            _mtemuFmt += _regs[1]->addr().value();
        }
        _mtemuFmt <<= ADDR_SIZE;

        if (_load && !_regs[0]->isRQ())
        {
            _mtemuFmt += _regs[0]->addr().value();
        }
        _mtemuFmt <<= WORD_SIZE;
        _mtemuFmt += _value;
    }
    else
    {
        _mtemuFmt <<= ADDR_SIZE;
        if (_load && !_regs[0]->isRQ())
        {
            _mtemuFmt += _regs[0]->addr().value();
        }
        _mtemuFmt <<= WORD_SIZE;
        _mtemuFmt += _value;
    }
}

int BinOpIn::GetNullPos() const noexcept
{
    return _nullPos;
}

BinOpIn::RegContainer& BinOpIn::GetRegs() noexcept
{
    return _regs;
}

const BinOpIn::RegContainer& BinOpIn::GetRegs() const noexcept
{
    return _regs;
}

void BinOpIn::Update(const BinOpIn::RegContainer& regs)
{
    _regs = regs;
    calcMtemu(true);
}

const BinOpIn::RegContainer& BinOpIn::GetTemplateRegs() const noexcept
{
    return _template_regs;
}
