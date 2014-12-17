#include <iostream>

#include "read.h"

using namespace std;

read_fld_func get_fld_reader(wire_t wire_type, fld_t fld_type) {
  switch (fld_type) {
  case FLD_STRING: return rf_string;
  case FLD_INT32: return rf_int32;
  case FLD_UINT32: return rf_uint32;
  case FLD_INT64: return rf_int64;
  case FLD_UINT64: return rf_uint64;
  default: return NULL;
  }
}

read_key_func get_key_reader(wire_t wire_type, fld_t fld_type) {
  return NULL;
}

#define FSET(val)  rb_funcall(obj, target_field_setter, 1, (val))
#define DEF_RF(type)  void rf_##type(ss_t& ss, VALUE obj, ID target_field_setter)

DEF_RF(string) {
  int32_t len = r_var_uint32(ss);
  FSET(rb_str_new(ss_read_chars(ss, len), len));
}

DEF_RF(int32) { FSET(INT2NUM(r_var_uint32(ss))); }
DEF_RF(uint32) { FSET(UINT2NUM(r_var_uint32(ss))); }
DEF_RF(int64) { FSET(LL2NUM(r_var_uint64(ss))); }
DEF_RF(uint64) { FSET(ULL2NUM(r_var_uint64(ss))); }

#define DEF_R_VARINT(bits)                                  \
  uint##bits##_t r_var_uint##bits(ss_t& ss) {               \
    uint##bits##_t val = 0;                                 \
      int sh_amt = 0;                                       \
      while (ss_more(ss)) {                                 \
        uint8_t b = ss_read_byte(ss);                       \
        val |= (((uint##bits##_t)b) & 127) << sh_amt;       \
        if ((b & 128) == 0)                                 \
          break;                                            \
        else                                                \
          sh_amt += 7;                                      \
      }                                                     \
      return val;                                           \
  }

DEF_R_VARINT(32)
DEF_R_VARINT(64)
