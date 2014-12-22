#include <iostream>

#include "read.h"
#include "read_write.h"
#include "encode.h"

using namespace std;

extern VALUE UTF_8_ENCODING;
extern VALUE FORCE_ID_ENCODING;
extern ID ID_CTOR;
extern ID ID_CALL;

// these macros are unhygienic, but that's ok since they are
// local to this file.
#define DEF_RF(type)  VALUE rf_##type(ss_t& ss, Fld* fld)

DEF_RF(INT32)    { return (INT2NUM          (r_varint32(ss)));  }
DEF_RF(UINT32)   { return (UINT2NUM         (r_varint32(ss)));  }
DEF_RF(INT64)    { return (LL2NUM           (r_varint64(ss)));  }
DEF_RF(UINT64)   { return (ULL2NUM          (r_varint64(ss)));  }
DEF_RF(SINT32)   { return (INT2NUM (zz_dec32(r_varint32(ss)))); }
DEF_RF(SINT64)   { return (LL2NUM  (zz_dec64(r_varint64(ss)))); }
DEF_RF(SFIXED32) { return (INT2NUM          (r_int32   (ss)));  }
DEF_RF(FIXED32)  { return (UINT2NUM         (r_int32   (ss)));  }
DEF_RF(SFIXED64) { return (LL2NUM           (r_int64   (ss)));  }
DEF_RF(FIXED64)  { return (ULL2NUM          (r_int64   (ss)));  }

DEF_RF(ENUM)     { return (INT2NUM          (r_varint32(ss)));  }

DEF_RF(FLOAT) {
  uint32_t v = r_int32(ss);
  return (DBL2NUM((double)REINTERPRET(float, v)));
}

DEF_RF(DOUBLE) {
  uint64_t v = r_int64(ss);
  return (DBL2NUM(REINTERPRET(double, v)));
}

DEF_RF(BOOL) {
  uint32_t v = r_varint32(ss);
  return (v == 0 ? Qfalse : Qtrue);
}

DEF_RF(STRING) {
  cout << "read string field" << endl;
  int32_t len = r_varint32(ss);
  VALUE rstr = rb_str_new(ss_read_chars(ss, len), len);
  return (rb_funcall(rstr, FORCE_ID_ENCODING, 1, UTF_8_ENCODING));
}

DEF_RF(BYTES) {
  int32_t len = r_varint32(ss);
  return (rb_str_new(ss_read_chars(ss, len), len));
}

DEF_RF(MESSAGE) {
  int32_t len = r_varint32(ss);
  ss_t tmp_ss = ss_substream(ss, len);
  Msg* embedded_msg = fld->embedded_msg;
  return (embedded_msg->read(embedded_msg, tmp_ss));
}

read_val_func get_fld_reader(wire_t wire_type, fld_t fld_type) {
  switch (fld_type) { TYPE_MAP(rf); default: cout << "get_fld_reader failed" << endl; return NULL; }
}

read_val_func get_key_reader(wire_t wire_type, fld_t fld_type) {
  //switch (fld_type) { TYPE_MAP(rk); default: return NULL; }
  return NULL;
}

#define INFLATE(val)  RTEST(fld->inflate) ? rb_funcall(fld->inflate, ID_CALL, 1, (val)) : (val)

VALUE read_obj(Msg* msg, ss_t& ss) {
  VALUE obj = rb_funcall(msg->target, ID_CTOR, 0);
  
  for (Fld& fld : msg->flds_to_enumerate)
    if (fld.label == LABEL_REPEATED)
      rb_funcall(obj, fld.target_field_setter, 1, rb_ary_new());

  while (ss_more(ss)) {
    uint32_t h = r_varint32(ss);
    fld_num_t fld_num = h >> 3;
    Fld* fld = msg->get_fld(msg, fld_num);
    if (fld->label != LABEL_REPEATED) {
      VALUE val = INFLATE(fld->read(ss, fld));
      rb_funcall(obj, fld->target_field_setter, 1, val);
    } else {
      if (fld->is_packed) {
        uint32_t byte_len = r_varint32(ss);
        ss_t tmp_ss = ss_substream(ss, byte_len);
        VALUE rArr = rb_funcall(obj, fld->target_field, 0);
        while (ss_more(tmp_ss))
          rb_ary_push(rArr, INFLATE(fld->read(tmp_ss, fld)));
      } else {
        VALUE val = fld->read(ss, fld);
        rb_ary_push(rb_funcall(obj, fld->target_field, 0), INFLATE(val));
      }
    }
  }
  return obj;
}
 
