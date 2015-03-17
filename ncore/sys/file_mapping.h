#ifndef NCORE_SYS_FILE_MAPPING_H_
#define NCORE_SYS_FILE_MAPPING_H_

#include <ncore/ncore.h>

namespace ncore
{
    

class FileMapping    
{
public:
    enum Mode
    {
        kRead,
        kReadWrite,
    };

public:
    FileMapping();
    ~FileMapping();

    bool init(const Mode mode, const uint64_t size);
    void fini();
    void * handle() const;


private:
    void * handle_;
};

    
}

#endif