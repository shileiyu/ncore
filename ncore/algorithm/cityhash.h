/*
Copy from Google's cityhash
url: https://code.google.com/p/cityhash/
*/

#ifndef NCORE_ALGORITHM_CITYHASH_H_
#define NCORE_ALGORITHM_CITYHASH_H_

#include <ncore/ncore.h>

namespace ncore
{


typedef std::pair<uint64_t, uint64_t> uint128_t;

inline uint64_t uint128_tLow64(const uint128_t& x) { return x.first; }
inline uint64_t uint128_tHigh64(const uint128_t& x) { return x.second; }

// Hash function for a byte array.
uint64_t CityHash64(const char *buf, size_t len);

// Hash function for a byte array.  For convenience, a 64-bit seed is also
// hashed into the result.
uint64_t CityHash64WithSeed(const char *buf, size_t len, uint64_t seed);

// Hash function for a byte array.  For convenience, two seeds are also
// hashed into the result.
uint64_t CityHash64WithSeeds(const char *buf, size_t len,
                           uint64_t seed0, uint64_t seed1);

// Hash function for a byte array.
uint128_t CityHash128(const char *s, size_t len);

// Hash function for a byte array.  For convenience, a 128-bit seed is also
// hashed into the result.
uint128_t CityHash128WithSeed(const char *s, size_t len, uint128_t seed);


}

#endif