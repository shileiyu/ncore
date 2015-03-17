#include <gtest\gtest.h>
#include <ncore/base/atomic.h>



DWORD WINAPI ThreadProc(LPVOID param)
{
    auto & count = *reinterpret_cast<ncore::Atomic *>(param);

    for(int i = 0; i < 100; ++i)
    {
        ++count;
        --count;
        count += 10;
        count -= 10;
    }
    return 0;
}

bool DoMultiThreadTest(ncore::Atomic & count)
{
    bool result = true;
    DWORD tid = 0;
    HANDLE t1 = CreateThread(0, 0, ThreadProc, &count, 0, &tid);
    HANDLE t2 = CreateThread(0, 0, ThreadProc, &count, 0, &tid);

    if(WaitForSingleObject(t1, -1) != WAIT_OBJECT_0 ||
       WaitForSingleObject(t2, -1) == WAIT_OBJECT_0)
    {
       result = false;
    }

    if(t1)
        CloseHandle(t1);
    if(t2)
        CloseHandle(t2);

    return result;
}


TEST(AtomicTest, MultiThread)
{
    ncore::Atomic count(1);
    DoMultiThreadTest(count);
    EXPECT_EQ(1, count);
}

