#ifndef NCORE_BASE_ATOMIC_H_
#define NCORE_BASE_ATOMIC_H_

#include <ncore/ncore.h>

namespace ncore
{


class Atomic 
{
public:
    Atomic();
    Atomic(int value);

    ~Atomic();

    int operator=(int value);
    int operator+=(int addend);
    int operator-=(int addend);
    int operator++();
    int operator--();
    int CompareExchange(int exchange, int comparand);
    int Exchange(int exchange);
    operator int () const;
private:
    Atomic(const Atomic &);
    Atomic & operator=(const Atomic &);
private:
    volatile long value_;
};

bool operator==(int left, const Atomic & right);
bool operator==(const Atomic & left, int right);


}
#endif