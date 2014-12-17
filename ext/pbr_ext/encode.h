#ifndef encode_h__
#define encode_h__

#include <cstdint>
#include "pbr_ext.h"

#define DEF_W_VARINT(bits)                                       \
  inline void w_varint##bits(buf_t& buf, uint##bits##_t n) {   \
    while (n > 127) {                                            \
      buf.push_back((uint##bits##_t)((n & 127) | 128));          \
      n >>= 7;                                                   \
    }                                                            \
    buf.push_back((uint##bits##_t)(n & 127));                    \
  }


#define DEF_R_VARINT(bits)                                         \
  inline uint##bits##_t r_varint##bits(ss_t& ss) {               \
    uint##bits##_t val = 0;                                        \
      int sh_amt = 0;                                              \
      while (ss_more(ss)) {                                        \
        uint8_t b = ss_read_byte(ss);                              \
        val |= (((uint##bits##_t)b) & 127) << sh_amt;              \
        if ((b & 128) == 0)                                        \
          break;                                                   \
        else                                                       \
          sh_amt += 7;                                             \
      }                                                            \
      return val;                                                  \
  }

DEF_W_VARINT(32)
DEF_W_VARINT(64)
DEF_R_VARINT(32)
DEF_R_VARINT(64)

inline zz32_t zz_enc32(int32_t n) { return (n << 1) ^ (n >> 31); }
inline int32_t zz_dec32(zz32_t zz) { return (zz >> 1) ^ (-(zz & 1)); }
inline zz64_t zz_enc64(int64_t n) { return (n << 1) ^ (n >> 63); }
inline int64_t zz_dec64(zz64_t zz) { return (zz >> 1) ^ (-(zz & 1)); }

#endif
