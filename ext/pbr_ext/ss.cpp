#include "ss.h"

ss_t ss_make(char *s, int len) {
  ss_t ss;
  ss.buf = reinterpret_cast<uint8_t*>(s);
  ss.pos = 0;
  ss.len = len;
  return ss;
}

ss_t ss_substream(ss_t& other, int len) {
  ss_t ss;
  ss.buf = other.buf + other.pos;
  ss.pos = 0;
  ss.len = len;
  other.pos += len;
  return ss;
}

uint8_t ss_read_byte(ss_t& ss) {
  return ss.buf[ss.pos++];
}

char *ss_read_chars(ss_t& ss, int32_t len) {
  char *s = reinterpret_cast<char*>(ss.buf + ss.pos);
  ss.pos += len;
  return s;
}
