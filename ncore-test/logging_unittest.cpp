#include <gtest\gtest.h>
#include <ncore/utils/logging.h>

using namespace ncore;

TEST(LoggingTest, BlackHole)
{
    Log::AlterLogDestination(kLogToBlackhole);

    LOG_VERBOSE << "Whatcha you want?";
    LOG_INFO << "Whatcha you want?";
    LOG_WARNING << "Whatcha you want?";
    LOG_ERROR << "Whatcha you want?";
}

