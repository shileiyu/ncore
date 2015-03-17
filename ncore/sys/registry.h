#ifndef NCORE_SYS_REGISTRY_H_
#define NCORE_SYS_REGISTRY_H_

#include <ncore/ncore.h>
#include <ncore/base/object.h>

namespace ncore
{

class Registry : public NonCopyableObject
{
public:
    static bool CreateAsNonPrivilege(const char * path);

    static bool Create(const char * path);

    static bool Delete(const char * path);

    Registry();

    ~Registry();

    bool Open(const char * name, const char * mode);

    void Close();

    bool Write(const char * name, uint32_t value);

    bool Write(const char * name, int32_t value);

    bool Write(const char * name, const std::string & value);

    bool Write(const char * name, const char * value);

    bool Read(const char * name, std::string & value);

    bool Read(const char * name, uint32_t & value);

    bool Read(const char * name, int32_t & value);

    bool Remove(const char * name);
private:
    RegKey key_;
};


}

#endif