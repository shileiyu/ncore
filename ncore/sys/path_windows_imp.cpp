#include <ncore/base/datetime.h>
#include <ncore/encoding/utf8.h>
#include <ncore/utils/handy.h>
#include <ncore/utils/karma.h>
#include "path.h"

namespace ncore
{


typedef 
BOOLEAN (APIENTRY *CreateSymbolicLinkW_T)
(LPCWSTR lpSymlinkFileName,
LPCWSTR lpTargetFileName,
DWORD dwFlags);

template<typename PROTO>
void GetKernelProc(LPCSTR name, PROTO & proc)
{
    static HMODULE kernel = GetModuleHandle(L"kernel32.dll");
    proc = reinterpret_cast<PROTO>(GetProcAddress(kernel, name));
    return;
}

static CreateSymbolicLinkW_T GetCreateSymbolicLinkW()
{
    CreateSymbolicLinkW_T proc = 0;
    GetKernelProc("CreateSymbolicLinkW", proc);
    return proc;
}

bool Path::Rename(const char * exists_file_name, 
                  const char * new_file_name)
{

    wchar_t exists_file_name16[kMaxPath16];
    wchar_t new_file_name16[kMaxPath16];

    if(!UTF8::Decode(exists_file_name, -1, exists_file_name16, kMaxPath16))
        return false;

    if(!UTF8::Decode(new_file_name, -1, new_file_name16, kMaxPath16))
        return false;

    DWORD flag = MOVEFILE_REPLACE_EXISTING;

    return ::MoveFileEx(exists_file_name16, new_file_name16, flag) != FALSE;
}

bool Path::Copy(const char * exists_file_name,
                const char * new_file_name,
                bool fail_if_exists)
{


    auto exists16 = Karma::FromUTF8(exists_file_name);
    auto new16 = Karma::FromUTF8(new_file_name);

    return ::CopyFile(exists16.data(), new16.data(), fail_if_exists) == TRUE;
}

bool Path::Copy(const std::string & exists_file_name,
                const std::string & new_file_name,
                bool fail_if_exists)
{
    return Copy(exists_file_name.data(), new_file_name.data(), fail_if_exists);
}

bool Path::Delete(const std::string & exists_file_name)
{
    return Delete(exists_file_name.data());
}

bool Path::Delete(const char * exists_file_name)
{

    wchar_t exists_file_name16[kMaxPath16];

    if(!UTF8::Decode(exists_file_name, -1, exists_file_name16, kMaxPath16))
        return false;

    DWORD attr = ::GetFileAttributes(exists_file_name16);
    if(attr == INVALID_FILE_ATTRIBUTES)
        return false;

    if(attr & FILE_ATTRIBUTE_DIRECTORY)
        return ::RemoveDirectory(exists_file_name16) != FALSE;

    return ::DeleteFile(exists_file_name16) != FALSE;
}

bool Path::CreateDirectoryRecursive(const std::string & name)
{
    return CreateDirectoryRecursive(name.data());
}

bool Path::CreateDirectoryRecursive(const char * name)
{

    bool result = false;
    DWORD attr = 0;

    wchar_t name16[kMaxPath16];    
    if(!UTF8::Decode(name, -1, name16, kMaxPath16))
        return false;

    attr = ::GetFileAttributes(name16);

    if(attr != INVALID_FILE_ATTRIBUTES)
        return attr & FILE_ATTRIBUTE_DIRECTORY ? true : false;

    wchar_t partical[kMaxPath16] = {0};
    for(size_t i = 0; i < kMaxPath16; ++i)
    {
        wchar_t ch = name16[i];

        if(ch == L'/' || ch == L'\\' || ch == 0)
        {
            attr = ::GetFileAttributes(partical);
            if(attr == INVALID_FILE_ATTRIBUTES)
                result = ::CreateDirectory(partical, 0) != FALSE;
            else
                result = attr & FILE_ATTRIBUTE_DIRECTORY ? true : false;

            if(!result || ch == 0)
                break;
            else
                ch = '/';
        }

        if(!(partical[i] = ch))
            break;
    }

    return result;
}

bool Path::DeleteDirectoryRecursive(const std::string & name)
{
    return DeleteDirectoryRecursive(name.data());
}

bool Path::DeleteDirectoryRecursive(const char * name)
{

    StringList files;
    StringList folders;

    if(!Walk(name, kAllDirectories, &files, &folders))
        return false;
    for(auto iter = files.begin(); iter != files.end(); ++iter)
        Delete(iter->data());
    for(auto iter = folders.begin(); iter != folders.end(); ++iter)
        Delete(iter->data());
    return Delete(name);
}

bool Path::isDirectoryExist(const std::string & name)
{
    return isDirectoryExist(name.data());
}

bool Path::isDirectoryExist(const char * name)
{
    wchar_t name16[kMaxPath16];    
    if(!UTF8::Decode(name, -1, name16, kMaxPath16))
        return false;

    DWORD attr = 0;
    attr = ::GetFileAttributes(name16);
    if((attr & FILE_ATTRIBUTE_DIRECTORY) && 
        attr != INVALID_FILE_ATTRIBUTES)
    {
        return true;
    }

    return false;
}

bool Path::isFileExist(const std::string & name)
{
    return isFileExist(name.data());
}

bool Path::isFileExist(const char * name)
{
    wchar_t name16[kMaxPath16];    
    if(!UTF8::Decode(name, -1, name16, kMaxPath16))
        return false;

    DWORD attr = 0;
    attr = GetFileAttributes(name16);
    if(!(attr & FILE_ATTRIBUTE_DIRECTORY) && 
         attr != INVALID_FILE_ATTRIBUTES)
    {
        return true;
    }

    return false;
}

bool Path::Walk(const char * path, SearchOption option,
                std::list<std::string> * files,
                std::list<std::string> * folders)
{
    //kMaxPattern should larger than kMaxPath 4 bytes at least.
    const size_t kMaxPattern = 512;
    wchar_t path16[kMaxPath16];
    wchar_t pattern[kMaxPattern];

    path16[0] = pattern[0] = 0;

    if(!UTF8::Decode(path, -1, path16, kMaxPath16))
        return false;

    Karma::Copy(pattern, path16);

    int pattern_length = wcslen(pattern);
    if(pattern_length == 0 || pattern_length > kMaxPath16)
        return false;

    wchar_t last = pattern[pattern_length - 1];
    if(last != L'/' && last != '\\')
        pattern[pattern_length++] = L'\\';

    pattern[pattern_length++] = L'*';
    pattern[pattern_length++] = 0;

    WIN32_FIND_DATA find_data = {0};
    HANDLE find_handle = 0;

    find_handle = FindFirstFile(pattern, &find_data);
    if(find_handle == INVALID_HANDLE_VALUE)
        return false;

    do
    {
        if(!wcscmp(find_data.cFileName, L".") ||
           !wcscmp(find_data.cFileName, L".."))
        {
            //忽略 .|.. 文件夹 
            continue;
        }

        char filename[kMaxPath8];
        if(!UTF8::Encode(find_data.cFileName, -1, filename, kMaxPath8))
            continue;

        const char * parts[] = { path, filename };
        std::string fullname = Path::JoinPath(parts, countof(parts));

        if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if(folders)
                folders->push_front(fullname);
            if(kAllDirectories == option)
                Walk(fullname.data(), option, files, folders);
        }
        else
        {
            if(files)
                files->push_front(fullname);
        }

    }while(FindNextFile(find_handle, &find_data));

    FindClose(find_handle);
    return true;
}

//std::string Path::GetPathFromFullName(const std::string & fullname)
//{
//    return GetPathFromFullName(fullname.data());
//}

std::string Path::GetPathFromFullName(const char * fullname)
{
    std::string path;
    if(!fullname)
        return path;

    auto last_backslash = std::strrchr(fullname, '\\');
    auto last_slash = std::strrchr(fullname, '/');
    auto slash = std::max(last_backslash, last_slash);

    if(!slash)
        return path;

    return path.assign(fullname, slash - fullname);
}

//std::string Path::GetFileName(const std::string & fullname)
//{
//    return GetFileName(fullname.data());
//}

std::string Path::GetFileName(const char * fullname)
{
    std::string filename;
    if(!fullname)
        return filename;

    auto last_backslash = std::strrchr(fullname, '\\');
    auto last_slash = std::strrchr(fullname, '/');
    auto slash = std::max(last_backslash, last_slash);

    if(!slash)
        return filename;

    return filename.assign(slash + 1);
}

//std::string Path::GetFileBaseName(const std::string & filename)
//{
//    return GetFileBaseName(filename);
//}

std::string Path::GetFileBaseName(const char * filename)
{
    std::string basename;
    if(!filename)
        return basename;

    auto last_backslash = std::strrchr(filename, '\\');
    auto last_slash = std::strrchr(filename, '/');
    auto slash = std::max(last_backslash, last_slash);
    auto last_dot = std::strrchr(filename, '.');

    if(!last_dot && !slash)
        return basename.assign(filename);

    if(last_dot < slash)
        return basename.assign(slash + 1);

    const char * begin = filename;
    if(slash)
        begin = slash + 1;
        
    if(last_dot >= begin)
        return basename.assign(begin, last_dot - begin);

    return basename;
}

////std::string Path::GetFileSuffix(const std::string & filename)
////{
////    return GetFileSuffix(filename.data());
////}

std::string Path::GetFileSuffix(const char * filename)
{
    std::string suffix;
    if(!filename)
        return suffix;

    auto last_backslash = std::strrchr(filename, '\\');
    auto last_slash = std::strrchr(filename, '/');
    auto slash = std::max(last_backslash, last_slash);
    auto last_dot = std::strrchr(filename, '.');

    if(!last_dot && !slash)
        return suffix;

    if(last_dot > slash)
        return suffix.assign(last_dot + 1);

    return suffix;
}

size_t Path::NormalizePath(const char * path, char * output, size_t size)
{
    if(path == 0 || output == 0 || size == 0)
        return 0;

    size_t max_processed_length = strlen(path) + 1;
    if(output == 0 || size < max_processed_length)
        return max_processed_length;

    const char * q = path;
    char * p = output;
    size_t out_len = 0;
    while(*p = *q++)
    {
        out_len = p - output;

        if(*p == L'\\')
            *p = '/';

        if(*p == L'/')
        {
            if(out_len >= 1)
            {
                if(!strncmp(p - 1, "//", 2))
                {
                    *p = 0;
                    continue;
                }
            }
            if(out_len >= 2)
            {
                if(!strncmp(p - 2, "/./", 3))
                {
                    *--p = 0;
                    continue;
                }
            }
            if(out_len >= 3)
            {
                if(!strncmp(p - 3, "/../", 4))
                {
                    p -= 3;
                    while(p > output)
                    {
                        if(*--p == L'/')
                        {
                            *++p = 0;
                            break;
                        }
                    }
                    continue;
                }
            }
        }
        ++p;
    }

    return p - output;
}

std::string Path::JoinPath(const char ** parts, size_t count)
{
    struct Part
    {
        const char * ptr;
        size_t size;
    };

    static const size_t kDefaultPartsCount = 64;
    static const size_t kDefaultPathLength = 300;

    std::string path;
    size_t path_len = 0;
    Part stack_lookup[kDefaultPartsCount];
    char stack_input[kDefaultPathLength] = {0};
    char stack_output[kDefaultPathLength] = {0};
    Part * lookup = 0;
    char * input = 0;
    char * output = 0;

    if (parts == 0 || count == 0)
        return path;

    lookup = count <= kDefaultPartsCount ? stack_lookup : new Part[count];
    if(lookup == 0)
        return path;

    for(size_t i = 0; i < count; ++i)
    {
        const char * part = parts[i];
        if(part == 0)
            continue;

        lookup[i].ptr = part;
        lookup[i].size = strlen(part);
        path_len += lookup[i].size + 1;
    }

    if(path_len <= kDefaultPathLength)
    {
        input = stack_input;
        output = stack_output;
        path_len = kDefaultPathLength;
    }
    else
    {
        input = new char[path_len];
        output = new char[path_len];
    }

    if(input && output)
    {
        size_t pos = 0;
        for(size_t i = 0; i < count; ++i)
        {
            const char * part = lookup[i].ptr;
            const size_t part_size = lookup[i].size;
            char * dst = input + pos;

            memcpy(dst, part, part_size);
            if (i != count -1)
                *(dst + part_size) = L'/';
            else
                *(dst + part_size) = L'\0';
            pos += part_size + 1;
        }
        NormalizePath(input, output, path_len);
        path = output;
    }

    if(lookup != 0 && lookup != stack_lookup)
        delete lookup;
    if(input != 0 && input != stack_input)
        delete input;
    if(output != 0 && output != stack_output)
        delete output;
    return path;
}

bool Path::Link(const char * link, const char * target, LinkMode mode)
{
    
    static auto TCreateSymbolLink = GetCreateSymbolicLinkW();

    wchar_t link16[kMaxPath16];
    wchar_t target16[kMaxPath16];

    if(!UTF8::Decode(link, -1, link16, kMaxPath16))
        return false;
    if(!UTF8::Decode(target, -1, target16, kMaxPath16))
        return false;

    if(mode == kPhysicLink)
    {
        return ::CreateHardLink(link16, target16, 0) != FALSE;
    }
    else if(mode == kSymbolLink)
    {
        DWORD flag = 0;
        if(isDirectoryExist(target))
            flag |= SYMBOLIC_LINK_FLAG_DIRECTORY;
        if(TCreateSymbolLink)
            return TCreateSymbolLink(link16, target16, flag) != FALSE;
        return false;
    }
    return false;
}

bool Path::DeleteOnReboot(const char * name)
{
    wchar_t name16[kMaxPath16];    
    if(!UTF8::Decode(name, -1, name16, kMaxPath16))
        return false;

    DWORD flag = MOVEFILE_DELAY_UNTIL_REBOOT;

    return ::MoveFileEx(name16, 0, flag) != FALSE;
}

bool Path::CreateDirectoryAsNonPrivilege(const std::string & name)
{
    return CreateDirectoryAsNonPrivilege(name.data());
}

bool Path::CreateDirectoryAsNonPrivilege(const char * name)
{

    if(!CreateDirectoryRecursive(name))
        return false;

    wchar_t name16[kMaxPath16];    
    if(!UTF8::Decode(name, -1, name16, kMaxPath16))
        return false;

    SE_OBJECT_TYPE seot = SE_FILE_OBJECT;
    SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
    PSECURITY_DESCRIPTOR sd = 0;
    PACL prev_dacl = 0;
    PACL curr_dacl = 0;
    PSID sid = 0;

    EXPLICIT_ACCESS ea = {0};
    ea.grfAccessPermissions = FILE_ALL_ACCESS;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;

    //S-1-1-0 refer to everyone.
    ConvertStringSidToSid(L"S-1-1-0", &sid);
    BuildTrusteeWithSid(&ea.Trustee, sid);

    bool result = false;

    if(!GetNamedSecurityInfo(name16, seot, si, 0, 0, &prev_dacl, 0, &sd))
        if(!SetEntriesInAcl(1, &ea, prev_dacl, &curr_dacl))
            if(!SetNamedSecurityInfo(name16, seot, si, 0, 0, curr_dacl, 0))
                result = true;
    if(sid)
        LocalFree(sid);
    if(sd)
        LocalFree(sd);
    if(curr_dacl)
        LocalFree(curr_dacl);

    return result;

}


}
