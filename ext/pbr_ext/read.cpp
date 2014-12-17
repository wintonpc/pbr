#include <iostream>

#include "read.h"
#include "read_write.h"
#include "encode.h"

using namespace std;

#define FSET(val)  rb_funcall(obj, target_field_setter, 1, (val))
#define DEF_RF(type)  void rf_##type(ss_t& ss, VALUE obj, ID target_field_setter)

DEF_RF(STRING) {
  int32_t len = r_varint32(ss);
  FSET(rb_str_new(ss_read_chars(ss, len), len));
}

DEF_RF(INT32)    { FSET(INT2NUM(          r_varint32(ss)));  }
DEF_RF(UINT32)   { FSET(UINT2NUM(         r_varint32(ss)));  }
DEF_RF(INT64)    { FSET(LL2NUM(           r_varint64(ss)));  }
DEF_RF(UINT64)   { FSET(ULL2NUM(          r_varint64(ss)));  }
DEF_RF(SINT32)   { FSET(INT2NUM( zz_dec32(r_varint32(ss)))); }
DEF_RF(SINT64)   { FSET(LL2NUM(  zz_dec64(r_varint64(ss)))); }
DEF_RF(SFIXED32) { FSET(INT2NUM(          r_int32(   ss)));  }
DEF_RF(FIXED32)  { FSET(UINT2NUM(         r_int32(   ss)));  }

read_fld_func get_fld_reader(wire_t wire_type, fld_t fld_type) {
  switch (fld_type) { TYPE_MAP(rf); default: return NULL; }
}

read_key_func get_key_reader(wire_t wire_type, fld_t fld_type) {
  //switch (fld_type) { TYPE_MAP(rk); default: return NULL; }
  return NULL;
}

