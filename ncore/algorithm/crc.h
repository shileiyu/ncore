#ifndef NCORE_ALGORITHM_CRC_H_
#define NCORE_ALGORITHM_CRC_H_

#include "hash.h"

namespace ncore
{


/*CRC16 start with 0*/
class CRC16Provider : public HashProvider
{
public:
    CRC16Provider();
    void Reset() override;
    void Update(const void * data, size_t size) override;
    void Final(Hash & hash) override;
    void Final(uint16_t & crc);
private:
    void init_crc16_tab(uint16_t *);
private:
    uint16_t crc_;
};

/*CRC32 start with 0xffffffff*/

class CRC32Provider : public HashProvider
{
public:
    CRC32Provider();
    void Reset() override;
    void Update(const void * data, size_t size) override;
    void Final(Hash & hash) override;
    void Final(uint32_t & hash);
private:
    void init_crc32_tab(uint32_t *);
private:
    uint32_t crc_;
};


}

#endif