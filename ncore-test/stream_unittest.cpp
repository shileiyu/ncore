#include <gtest\gtest.h>
#include <ncore/base/stream.h>

using namespace ncore;

TEST(StreamTest, Seek)
{
    MemoryStream s;
    uint32_t transfered;

    s.Seek(0x100000, StreamPosition::kBegin);
    EXPECT_EQ(0x100000, s.Tell());
    s.Seek(0x10, StreamPosition::kCurrent);
    EXPECT_EQ(0x100010, s.Tell());
    s.Seek(-0x10, StreamPosition::kCurrent);
    EXPECT_EQ(0x100000, s.Tell());
    s.Seek(0, StreamPosition::kEnd);
    EXPECT_EQ(0, s.Tell());
    s.Write("1234567890abcdef", 16, transfered);
    s.Seek(0, StreamPosition::kEnd);
    EXPECT_EQ(16, s.Tell());
}

TEST(StreamTest, ReadWrite)
{
    char wb[0x1000];
    char rb[0x1000];
    uint32_t transfered;

    MemoryStream s;
    s.Write("1234567890abcdef", 16, transfered);
    s.Write(wb, sizeof(wb), transfered);
    EXPECT_EQ(sizeof(wb) + 16, s.Tell());
    s.Seek(16, StreamPosition::kBegin);
    s.Read(rb, sizeof(rb), transfered);
    EXPECT_EQ(0, memcmp(wb, rb, 0x1000));
    s.Read(rb, sizeof(rb), transfered);
    EXPECT_EQ(0, transfered);
}

