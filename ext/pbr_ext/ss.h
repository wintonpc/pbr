#ifndef ss_h__
#define ss_h__

#include <cstdint>

typedef struct StringStream {
  uint8_t* buf;
  int pos;
  int len;
} ss_t;

ss_t ss_make(char* s, int len);
uint8_t ss_read_byte(ss_t& ss);
char* ss_read_chars(ss_t& ss, int32_t len);
#define ss_more(ss)  (ss).pos < (ss).len

#endif