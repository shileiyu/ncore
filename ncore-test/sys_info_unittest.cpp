#include <gtest/gtest.h>
#include <ncore/sys/sys_info.h>

using namespace ncore;

TEST(SysInfo, CommonAppDataPath)
{
    auto path = SysInfo::CommonAppDataPath();
}