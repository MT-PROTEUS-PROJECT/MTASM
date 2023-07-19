#pragma once

#include "gtest/gtest.h"
#include <crtdbg.h>

class MemoryLeakDetector
{
public:
    MemoryLeakDetector()
    {
        _CrtMemCheckpoint(&memState_);
    }

    ~MemoryLeakDetector()
    {
#ifdef _DEBUG
        _CrtMemState stateNow, stateDiff;
        _CrtMemCheckpoint(&stateNow);
        int diffResult = _CrtMemDifference(&stateDiff, &memState_, &stateNow);
        if (diffResult)
            reportFailure(stateDiff.lSizes[1]);
#endif
    }

private:
    void reportFailure(size_t unfreedBytes)
    {
        FAIL() << "Memory leak of " << unfreedBytes << " byte(s) detected.";
    }
#ifdef _DEBUG
    _CrtMemState memState_;
#endif
};
