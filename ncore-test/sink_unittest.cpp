#include <gtest/gtest.h>
#include <ncore/utils/sink.h>

using namespace ncore;

static bool signal_received = false;

struct TestSignal
{
    TestSignal()
    {
        x = 0;
    }

    TestSignal(int val) 
    { 
        x = val; 
    }

    int x;
};

class TestLisener : public Sink::SlotListenerImp<TestSignal>
{
public:
    void OnNotify(const TestSignal & signal)
    {
        ASSERT_EQ(signal.x, 0xdeadbeef);
        signal_received = true;
    }
};

TEST(Sink, Generic)
{
    Sink sink;
    TestLisener l;

    sink.Register(&l);
    sink.Pour(Sink::Wrap(TestSignal(0xdeadbeef)));
    sink.Shake();
    ASSERT_TRUE(signal_received);
}
