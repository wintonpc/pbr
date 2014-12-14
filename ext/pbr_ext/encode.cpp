#include "encode.h"

zz_t zz_enc32(int n) {
  return (n << 1) ^ (n >> 31);
}

int zz_dec32(zz_t zz) {
  return (zz >> 1) ^ (-(zz & 1));
}
