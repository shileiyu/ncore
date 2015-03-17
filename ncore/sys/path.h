#ifndef NCORE_SYS_PATH_H_
#define NCORE_SYS_PATH_H_

#include <ncore/ncore.h>


namespace ncore
{

class DateTime;


class Path
{
public:
    enum SearchOption
    {
        kTopDirectoryOnly,
        kAllDirectories,
    };

    enum LinkMode
    {
        kPhysicLink,
        kSymbolLink,
    };

    typedef std::list<std::string> StringList;

public:
    static bool Rename(const char * exists_file_name, 
                       const char * new_file_name);

    static bool Copy(const char * exists_file_name,
                     const char * new_file_name,
                     bool fail_if_exists);

    static bool Copy(const std::string & exists_file_name,
                     const std::string & new_file_name,
                     bool fail_if_exists);

    static bool Delete(const std::string & exists_file_name);

    static bool Delete(const char * exists_file_name);

    static bool CreateDirectoryRecursive(const std::string & name);

    static bool CreateDirectoryRecursive(const char * name);

    static bool DeleteDirectoryRecursive(const std::string & name);

    static bool DeleteDirectoryRecursive(const char * name);

    static bool isDirectoryExist(const std::string & name);

    static bool isDirectoryExist(const char * name);

    static bool isFileExist(const std::string & name);

    static bool isFileExist(const char * name);

    static bool Walk(const char * path, SearchOption option,
                     StringList * files, StringList * folders);

    /* "...\foo.bar" => "...\" */
    static std::string GetPathFromFullName(const char * fullname);
    /* "...\foo.bar" => "foo.bar" */
    static std::string GetFileName(const char * fullname);
    /* "...\foo.bar" => "foo" */
    static std::string GetFileBaseName(const char * filename);
    /* "...\foo.bar" => "bar" */
    static std::string GetFileSuffix(const char * filename);

    static size_t NormalizePath(const char * path, char * output, size_t size);

    template<size_t Size>
    static std::string JoinPath(const char * (&dst)[Size])
    {
        return JoinPath(dst, Size);
    }

    static std::string JoinPath(const char ** parts, size_t count);

    static bool Link(const char * link, const char * target, LinkMode mode);

    static bool DeleteOnReboot(const char * name);

    static bool CreateDirectoryAsNonPrivilege(const std::string & name);

    static bool CreateDirectoryAsNonPrivilege(const char * name);
};


}

#endif