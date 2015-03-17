#include <gtest\gtest.h>
#include <ncore/base/buffer.h>

using namespace ncore;

TEST(BufferTest, Generic)
{
    Buffer a;
    EXPECT_EQ(0, a.capacity());
    EXPECT_EQ(NULL, a.data());

    Buffer b(16);
    EXPECT_EQ(16, b.capacity());
    EXPECT_EQ(true, b.data() != NULL);
}

TEST(BufferTest, Copy)
{
    Buffer foo(16);
    foo.SetContent("1234567890abcdef", 16);
    Buffer a(foo);
    Buffer b = foo;
    EXPECT_EQ(16, a.capacity());
    EXPECT_EQ(16, b.capacity());
    EXPECT_EQ(true, a.data() != foo.data());
    EXPECT_EQ(true, b.data() != foo.data());
    EXPECT_EQ(0, memcmp(a.data(), "1234567890abcdef", 16));
    EXPECT_EQ(0, memcmp(b.data(), "1234567890abcdef", 16));
}

TEST(BufferTest, Steal)
{
    Buffer foo(16);
    foo.SetContent("1234567890abcdef", 16);
    Buffer a(std::move(foo));
    EXPECT_EQ(0, foo.capacity());
    EXPECT_EQ(NULL, foo.data());
    EXPECT_EQ(0, memcmp(a.data(), "1234567890abcdef", 16));
    Buffer b;
    b = std::move(a);
    EXPECT_EQ(0, a.capacity());
    EXPECT_EQ(NULL, a.data());
    EXPECT_EQ(0, memcmp(b.data(), "1234567890abcdef", 16));
}

TEST(FixedBufferTest, Generic)
{
    FixedBuffer<16> a;
    FixedBuffer<16> b;
    a.SetContent("1234567890abcdef",16);
    b.SetContent("1234567890abcdef",17);
    EXPECT_EQ(true, a == b);
    EXPECT_EQ('1', a[0]);
    EXPECT_EQ('f', b[15]);
    EXPECT_EQ(true, (char *)a != NULL);
    EXPECT_EQ(true, (const char *)a != NULL);
}

