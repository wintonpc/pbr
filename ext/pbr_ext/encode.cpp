#include "encode.h"

zz_t zz_enc32(int32_t n) {
  return (n << 1) ^ (n >> 31);
}

int32_t zz_dec32(zz_t zz) {
  return (zz >> 1) ^ (-(zz & 1));
}

zz64_t zz_enc64(int64_t n) {
  return (n << 1) ^ (n >> 63);
}

int64_t zz_dec64(zz64_t zz) {
  return (zz >> 1) ^ (-(zz & 1));
}
