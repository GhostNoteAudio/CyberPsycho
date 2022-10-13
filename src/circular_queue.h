#pragma once
#include <Arduino.h>

template<class T, int S>
class CircularQueue
{
    T data[S];
    int idxWrite;
    int idxRead;
    int size;

public:
    CircularQueue()
    {
        idxWrite = 0;
        idxRead = 0;
        size = 0;
    }

    void Push(T item)
    {
        data[idxWrite] = item;
        idxWrite = (idxWrite + 1) % S;
        size++;
        if (size > S)
        {
            Serial.println("QUEUE OVERFLOW!");
            size--;
            idxRead = (idxRead + 1) % S;
        }
    }

    T Pop()
    {
        if (size <= 0)
        {
            Serial.println("QUEUE UNDERFLOW!");
            return T();
        }
        
        T item = data[idxRead];
        idxRead = (idxRead + 1) % S;
        size--;
        return item;
    }

    T Front()
    {
        if (size <= 0)
        {
            Serial.println("QUEUE UNDERFLOW!");
            return T();
        }
        
        T item = data[idxRead];
        return item;
    }

    int Size()
    {
        return size;
    }

    bool Empty()
    {
        return size == 0;
    }
};
