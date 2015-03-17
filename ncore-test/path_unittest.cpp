#include <gtest/gtest.h>
#include <ncore/sys/path.h>

using namespace ncore;

TEST(FileTest, NormalizeFullName)
{
    struct
    {
        char * name1;
        char * name2;
    } gm[] = 
    {
        {
            ".\\abc",
            "./abc", 
        },
        {
            "..\\abc",
            "../abc", 
        },
        {
            "\\.\\abc",
            "/abc", 
        },
        {
            "a/b\\.\\c\\../.\\c/../d\\/\\//.\\./.\\e",
            "a/b/d/e", 
        },
        {
            "a/a/../../b",
            "b", 
        },
        {   /*double slash*/
            "c:\\\\abc",
            "c:/abc",
        }
    };
    char out[512];

    for(auto i = 0; i < sizeof(gm) / sizeof(gm[0]); ++i)
    {
        Path::NormalizePath(gm[i].name1, out, 512);
        EXPECT_EQ(0, strcmp(out, gm[i].name2));
    }
}

TEST(FileTest, JoinPath)
{
    std::string output;

    // end slash
    std::string target_end_slash("c:/a/b/");
    const char * parts_end_slash_1[] = {"c:", "a", "b/"};
    output = Path::JoinPath(parts_end_slash_1, 3);
    EXPECT_TRUE(target_end_slash == output);

    const char * parts_end_slash_2[] = {"c:", "a", "b\\"};
    output = Path::JoinPath(parts_end_slash_2, 3);
    EXPECT_TRUE(target_end_slash == output);

    const char * parts_end_slash_3[] = {"c:", "a", "b"};
    output = Path::JoinPath(parts_end_slash_3, 3);
    EXPECT_TRUE(target_end_slash != output);

    // end no slash
    std::string target_end_noslash("c:/a/b");
    const char * parts_end_noslash_1[] = {"c:", "a", "b"};
    output = Path::JoinPath(parts_end_noslash_1, 3);
    EXPECT_TRUE(target_end_noslash == output);

    const char * parts_end_noslash_2[] = {"c:", "a", "b/"};
    output = Path::JoinPath(parts_end_noslash_2, 3);
    EXPECT_TRUE(target_end_noslash != output);

    const char * parts_end_noslash_3[] = {"c:", "a", "b\\"};
    output = Path::JoinPath(parts_end_noslash_3, 3);
    EXPECT_TRUE(target_end_noslash != output);
}

TEST(FileTest, CreateNonPrivilegeDirectory)
{
    auto name = "C:/Program Files (x86)/PandoraManager";
    Path::CreateDirectoryAsNonPrivilege(name);
}
