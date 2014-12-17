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

char* ss_read_chars(ss_t& ss, int32_t len) {
  char* s = (char*)(ss.buf + ss.pos);
  ss.pos += len;
  return s;
}
