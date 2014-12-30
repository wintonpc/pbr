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
#define DEF_RF(type)  VALUE rf_##type(ss_t& ss, Fld& fld)

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
  Msg& embedded_msg = *fld.embedded_msg;
  return embedded_msg.read(embedded_msg, tmp_ss);
}

read_val_func get_fld_reader(fld_t fld_type) {
  switch (fld_type) {
    TYPE_MAP(rf);
    default:
      rb_raise(rb_eStandardError, "I don\'t know how to read field type %d", fld_type);
  }
}

read_val_func get_key_reader(fld_t fld_type) {
  rb_raise(rb_eStandardError, "I don\'t know how to read field type %d", fld_type);
}

#define INFLATE(val)  RTEST(fld.inflate) ? rb_funcall(fld.inflate, ID_CALL, 1, (val)) : (val)

void ss_skip(ss_t& ss, int32_t n) {
  ss.pos += n;
}

void skip(ss_t& ss, wire_t wire_type) {
  uint32_t len;
  switch (wire_type) {
  case WIRE_VARINT: 
    r_varint64(ss);
    break;
  case WIRE_64BIT:
    ss_skip(ss, sizeof(int64_t));
    break;
  case WIRE_LENGTH_DELIMITED:
    len = r_varint32(ss);
    ss_skip(ss, len);
    break;
  case WIRE_32BIT:
    ss_skip(ss, sizeof(int32_t));
    break;
  }
}

VALUE read_obj(Msg& msg, ss_t& ss) {
  //cerr << "read_obj " << msg.name << endl;
  VALUE obj = rb_funcall(msg.target, ID_CTOR, 0);
  
  for (Fld& fld : msg.flds_to_enumerate)
    if (fld.label == LABEL_REPEATED)
      rb_funcall(obj, fld.target_field_setter, 1, rb_ary_new());

  while (ss_more(ss)) {
    uint32_t h = r_varint32(ss);
    fld_num_t fld_num = h >> 3;
    //cerr << "# " << fld_num << "  " << ss.pos << "/" << ss.len << endl;
    Fld* fld_ptr = msg.find_fld(msg, fld_num);

    if (fld_ptr == NULL) {
      cerr << "skipping unrecognized field " << msg.name << "." << fld_num << endl;
      wire_t wire_type = h & 7;
      skip(ss, wire_type);
      continue;
    }

    Fld& fld = *fld_ptr;

    if (fld.label != LABEL_REPEATED) {
      VALUE val = INFLATE(fld.read(ss, fld));
      rb_funcall(obj, fld.target_field_setter, 1, val);
    } else {
      if (fld.is_packed) {
        uint32_t byte_len = r_varint32(ss);
        ss_t tmp_ss = ss_substream(ss, byte_len);
        VALUE rArr = rb_funcall(obj, fld.target_field_getter, 0);
        while (ss_more(tmp_ss))
          rb_ary_push(rArr, INFLATE(fld.read(tmp_ss, fld)));
      } else {
        VALUE val = fld.read(ss, fld);
        rb_ary_push(rb_funcall(obj, fld.target_field_getter, 0), INFLATE(val));
      }
    }
  }
  return obj;
}
 
