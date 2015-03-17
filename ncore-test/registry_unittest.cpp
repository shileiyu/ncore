#include <gtest/gtest.h>
#include <ncore/sys/registry.h>

using namespace ncore;

TEST(Registry, Generic) 
{
    std::string str = "";
    int i = 0;
    Registry reg;
    reg.Create("HKCU\\Software\\Test");
    reg.Open("HKCU\\Software\\Test", "rw");
    reg.Write("string", "helloworld");
    reg.Write("int", 1);
    reg.Read("string", str);
    reg.Read("int", i);
    reg.Remove("int");
    reg.Close();
    reg.Delete("HKCU\\Software\\Test");
}