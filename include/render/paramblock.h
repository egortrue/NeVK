#pragma once
#include <vector>
#include <array>

template <class T>
struct ParamBlockFactory
{
    struct ParamBlock : public T
    {
    };

    std::array<ParamBlock, 3> params;

    void init()
    {
    }
    ParamBlock& getParamBlock(uint32_t index)
    {
        params[index % 3];
    }
};
