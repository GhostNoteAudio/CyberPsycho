#pragma once

template<int T>
class IOBuffer
{
public:
    const int Size = T;
    uint8_t Gate[4][T] = {{0}};
    uint16_t Cv[4][T] = {{0}};
    uint16_t Mod[4][T] = {{0}};
    uint16_t Out[4][T] = {{0}};
};
