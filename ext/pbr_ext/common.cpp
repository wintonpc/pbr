#include "common.h"

ss_t ss_make(char* s, int len) {
  ss_t ss;
  ss.buf = (uint8_t*)s;
  ss.pos = 0;
  ss.len = len;
  return ss;
}

uint8_t ss_read_byte(ss_t& ss) {
  return ss.buf[ss.pos++];
}
