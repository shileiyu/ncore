#include "atomic.h"

namespace ncore
{


Atomic::~Atomic()
{

}

Atomic::Atomic() 
{
    Exchange(0);
}

Atomic::Atomic( int value)
{
    Exchange(value);
}


int Atomic::operator=( int value)
{
    return Exchange(value);
}

int Atomic::operator+=( int addend)
{
    auto initial = _InterlockedExchangeAdd(&value_, addend);
    return initial + addend;
}

int Atomic::operator-=( int addend)
{
    return operator+=(-addend);
}

int Atomic::operator++()
{
    return _InterlockedIncrement(&value_);
}

int Atomic::operator--()
{
    return _InterlockedDecrement(&value_);
}

int Atomic::CompareExchange(int exchange, int comparand)
{
    return _InterlockedCompareExchange(&value_, exchange, comparand);
}

int Atomic::Exchange(int exchange)
{
    return _InterlockedExchange(&value_, exchange);
}

Atomic::operator int () const
{
    return value_;
}

bool operator==(int left, const Atomic & right)
{
    return left == (int)right;
}

bool operator==(const Atomic & left, int right)
{
    return (int)left == right;
}


}