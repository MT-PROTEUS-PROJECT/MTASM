#pragma once

#include <fstream>
#include <queue>
#include <memory>

#include "Expressions.h"

class Publisher final
{
private:
    std::queue<std::shared_ptr<Expression>> _qexpr;
    std::ofstream _out;

private:
    Publisher();

    void Write();

public:
    Publisher(const Publisher &) = delete;
    Publisher &operator=(const Publisher &) = delete;

    Publisher(Publisher &&) = delete;
    Publisher &operator=(Publisher &&) = delete;

    ~Publisher();

public:
    void Push(const std::shared_ptr<Expression> &expr);

    static Publisher *GetInstance();
};
