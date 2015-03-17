#include <ncore/encoding/utf8.h>
#include <ncore/utils/handy.h>
#include <ncore/utils/karma.h>
#include "registry.h"

namespace ncore
{


static const char * kCurrentUser = "HKCU";
static const char * kLocalMachine = "HKLM";
static const size_t kMaxRegistryPath = 1024;


static DWORD ParseMode(const char * mode)
{
    if(!mode)
        return 0;

    DWORD access = 0;
    char ch = *mode;
    while(ch)
    {
        if(ch == 'r')
            access |= KEY_READ;
        else if(ch == 'w')
            access |= KEY_WRITE;
        ch = *++mode;
    }
    return access;
}


static HKEY SplitPath(const char * path, const char *& sub)
{
    HKEY root = 0;
    sub = path;
    if(!strncmp(path, "HKCU", 4))
    {
        root = HKEY_CURRENT_USER;
        sub = path + 4;
    }
    else if(!strncmp(path, "HKLM", 4))
    {
        root = HKEY_LOCAL_MACHINE;
        sub = path + 4;
    }
    if(*sub == '\\')
        ++sub;
    return root;
}

Registry::Registry()
    :key_(0)
{
    ;
}

Registry::~Registry()
{
}

bool Registry::Write(const char * name, const char * value)
{

    wchar_t name16[kMaxRegistryPath];
    UTF8::Decode(name, -1, name16, countof(name16));

    std::wstring value16 = Karma::FromUTF8(value);

    auto data = reinterpret_cast<LPCBYTE>(value16.data());
    DWORD size = (value16.length() + 1) * sizeof(wchar_t);
    return !RegSetValueEx(key_, name16, 0, REG_SZ, data, size);
}

bool Registry::Write(const char * key, const std::string& value)
{
    return Write(key, value.data());
}

bool Registry::Write(const char * name, uint32_t value)
{

    wchar_t name16[kMaxRegistryPath];
    UTF8::Decode(name, -1, name16, countof(name16));

    auto data = reinterpret_cast<LPCBYTE>(&value);
    auto size = sizeof(value);
    return !RegSetValueEx(key_, name16, 0, REG_DWORD, data, size);
}

bool Registry::Write(const char * name, int32_t value)
{

    wchar_t name16[kMaxRegistryPath];
    UTF8::Decode(name, -1, name16, countof(name16));

    auto data = reinterpret_cast<LPCBYTE>(&value);
    auto size = sizeof(value);
    return !RegSetValueEx(key_, name16, 0, REG_DWORD, data, size);
}

bool Registry::Read(const char * name, uint32_t & value)
{

    wchar_t name16[kMaxRegistryPath];
    UTF8::Decode(name, -1, name16, countof(name16));

    auto data = reinterpret_cast<LPBYTE>(&value);
    DWORD size = sizeof(value);
    return !RegQueryValueEx(key_, name16, 0, 0, data, &size);
}

bool Registry::Read(const char * name, int32_t & value)
{

    wchar_t name16[kMaxRegistryPath];
    UTF8::Decode(name, -1, name16, countof(name16));

    auto data = reinterpret_cast<LPBYTE>(&value);
    DWORD size = sizeof(value);
    return !RegQueryValueEx(key_, name16, 0, 0, data, &size);
}

bool Registry::Read(const char * name, std::string & value)
{
    bool bret = false;
    DWORD _size = 0;
    DWORD _type = REG_BINARY;

    wchar_t name16[kMaxRegistryPath];
    UTF8::Decode(name, -1, name16, countof(name16));

    bret = !RegQueryValueEx(key_, name16, 0, &_type, 0, &_size);
    bret = _type == REG_SZ;
    if(bret)
    {
        auto _data = new BYTE[_size / 4 * 4 + 4];
        _data[_size] = 0;
        bret = !RegQueryValueEx(key_, name16, 0, 0, _data, &_size);
        if(bret)
        {
            auto _unicode = reinterpret_cast<const wchar_t*>(_data);
            value = Karma::ToUTF8(_unicode);
        }
        delete[] _data;
    }
    return bret;
}

bool Registry::Remove(const char * name)
{
    wchar_t name16[kMaxRegistryPath];
    UTF8::Decode(name, -1, name16, countof(name16));
    return !RegDeleteValue(key_, name16);
}

bool Registry::Open(const char * path, const char * mode)
{
    if(!path || !mode)
        return false;

    const char * subpath = 0;
    HKEY root = SplitPath(path, subpath);
    DWORD access = ParseMode(mode);

    wchar_t subpath16[kMaxRegistryPath];
    UTF8::Decode(subpath, -1, subpath16, countof(subpath16));
    return !RegOpenKeyEx(root, subpath16, 0, access, &key_);
}

void Registry::Close()
{
    if(key_)
    {
        ::RegCloseKey(key_);
        key_ = 0;
    }
}

bool CreateKeyInternal(const char * path, DWORD access, HKEY * key)
{
    if(!path)
        return false;

    const char * subpath = 0;
    HKEY root = SplitPath(path, subpath);
    DWORD option = REG_OPTION_NON_VOLATILE;

    wchar_t subpath16[kMaxRegistryPath];
    UTF8::Decode(subpath, -1, subpath16, countof(subpath16));

    if(::RegCreateKeyEx(root, subpath16, 0, 0, option, access, 0, key, 0))
        return false;

    return true;
}

bool Registry::CreateAsNonPrivilege(const char * path)
{
    bool result = false;
    HKEY key = 0;
    SE_OBJECT_TYPE seot = SE_REGISTRY_KEY;
    SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
    PSECURITY_DESCRIPTOR sd = 0;
    PACL prev_dacl = 0;
    PACL curr_dacl = 0;
    PSID sid = 0;

    EXPLICIT_ACCESS ea = {0};
    ea.grfAccessPermissions = KEY_ALL_ACCESS;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;

    //S-1-1-0 refer to everyone.
    ConvertStringSidToSid(L"S-1-1-0", &sid);
    BuildTrusteeWithSid(&ea.Trustee, sid);

    if(CreateKeyInternal(path, KEY_ALL_ACCESS, &key))
        if(!GetSecurityInfo(key, seot, si, 0, 0, &prev_dacl, 0, &sd))
            if(!SetEntriesInAcl(1, &ea, prev_dacl, &curr_dacl))
                if(!SetSecurityInfo(key, seot, si, 0, 0, curr_dacl, 0))
                    result = true;
    if(key)
        RegCloseKey(key);
    if(sid)
        LocalFree(sid);
    if(sd)
        LocalFree(sd);
    if(curr_dacl)
        LocalFree(curr_dacl);

    return result;
}

bool Registry::Create(const char * path)
{
    HKEY key = 0;
    if(!CreateKeyInternal(path, KEY_WRITE, &key))
        return false;
    RegCloseKey(key);
    return true;
}

bool Registry::Delete(const char * path)
{
    if(!path)
        return false;

    const char * subpath = 0;
    HKEY root = SplitPath(path, subpath);

    wchar_t subpath16[kMaxRegistryPath];
    UTF8::Decode(subpath, -1, subpath16, countof(subpath16));
    return !::RegDeleteKey(root, subpath16);
}


}