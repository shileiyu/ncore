#include <functional>
#include <gtest/gtest.h>
#include <ncore/utils/invoker.h>



class Counter
{
public:

    Counter() : z_(0) {}

    static void Inc(Counter * c)
    {
        c->z_ += 1;
        return;
    }

    static void Plus(Counter * c, int x)
    {
        c->z_ += x;
        return;
    }

    static void PlusByRef(Counter * c, const int & x)
    {
        c->z_ += x;
        return;
    }

    void Dec()
    {
        z_ -= 1;
    }

    void Minus(int x)
    {
        z_ -= x;
    }

    void MinusByRef(const int & x)
    {
        z_ -= x;
    }

    int Test(int)
    {
        return 1234;
    }

public:
    int z_;
};

template<typename Func>
void Wrap(Func && f)
{
    return;
}

TEST(Invoker, Generic)
{
    ncore::AsyncInvoker invoker;

    Counter c;
    int x = 3;


    auto task = invoker.QueueTask(std::bind(&Counter::Test, &c, 1));
    invoker.QueueTask(std::bind(&Counter::Inc, &c));
    invoker.QueueTask(std::bind(&Counter::Plus, &c, 1));
    invoker.QueueTask(std::bind(&Counter::PlusByRef, &c, x));
    invoker.QueueTask(std::bind(&Counter::Dec, &c));
    invoker.QueueTask(std::bind(&Counter::Minus, &c, 1));
    invoker.QueueTask(std::bind(&Counter::MinusByRef, &c, x));

    while(invoker.ProcessOne());

    int r = task->Result();

    EXPECT_EQ(c.z_, 0);
    
}