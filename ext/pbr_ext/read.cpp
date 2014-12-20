#include <iostream>

#include "read.h"
#include "read_write.h"
#include "encode.h"

using namespace std;

extern VALUE UTF_8_ENCODING;
extern VALUE FORCE_ID_ENCODING;

// these macros are unhygienic, but that's ok since they are
// local to this file.
#define FSET(val)  rb_funcall(obj, fld->target_field_setter, 1, (val))
#define DEF_RF(type)  void rf_##type(ss_t& ss, VALUE obj, Fld* fld)

DEF_RF(INT32)    { FSET(INT2NUM          (r_varint32(ss)));  }
DEF_RF(UINT32)   { FSET(UINT2NUM         (r_varint32(ss)));  }
DEF_RF(INT64)    { FSET(LL2NUM           (r_varint64(ss)));  }
DEF_RF(UINT64)   { FSET(ULL2NUM          (r_varint64(ss)));  }
DEF_RF(SINT32)   { FSET(INT2NUM (zz_dec32(r_varint32(ss)))); }
DEF_RF(SINT64)   { FSET(LL2NUM  (zz_dec64(r_varint64(ss)))); }
DEF_RF(SFIXED32) { FSET(INT2NUM          (r_int32   (ss)));  }
DEF_RF(FIXED32)  { FSET(UINT2NUM         (r_int32   (ss)));  }
DEF_RF(SFIXED64) { FSET(LL2NUM           (r_int64   (ss)));  }
DEF_RF(FIXED64)  { FSET(ULL2NUM          (r_int64   (ss)));  }

DEF_RF(ENUM)     { FSET(INT2NUM          (r_varint32(ss)));  }

DEF_RF(FLOAT) {
  uint32_t v = r_int32(ss);
  FSET(DBL2NUM((double)REINTERPRET(float, v)));
}

DEF_RF(DOUBLE) {
  uint64_t v = r_int64(ss);
  FSET(DBL2NUM(REINTERPRET(double, v)));
}

DEF_RF(BOOL) {
  uint32_t v = r_varint32(ss);
  FSET(v == 0 ? Qfalse : Qtrue);
}

DEF_RF(STRING) {
  int32_t len = r_varint32(ss);
  VALUE rstr = rb_str_new(ss_read_chars(ss, len), len);
  FSET(rb_funcall(rstr, FORCE_ID_ENCODING, 1, UTF_8_ENCODING));
}

DEF_RF(BYTES) {
  int32_t len = r_varint32(ss);
  FSET(rb_str_new(ss_read_chars(ss, len), len));
}

DEF_RF(MESSAGE) {
  int32_t len = r_varint32(ss);
  ss_t tmp_ss = ss_substream(ss, len);
  Msg* embedded_msg = fld->embedded_msg;
  FSET(embedded_msg->read(embedded_msg, tmp_ss));
}

read_fld_func get_fld_reader(wire_t wire_type, fld_t fld_type) {
  switch (fld_type) { TYPE_MAP(rf); default: return NULL; }
}

read_fld_func get_key_reader(wire_t wire_type, fld_t fld_type) {
  //switch (fld_type) { TYPE_MAP(rk); default: return NULL; }
  return NULL;
}

