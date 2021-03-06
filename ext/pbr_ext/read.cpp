#include <iostream>

#include "read.h"
#include "read_write.h"
#include "encode.h"

using namespace std;

void ss_skip(ss_t& ss, int32_t n);
void skip(ss_t& ss, wire_t wire_type);

// conventional variables used in this file
// ss_t& ss  - the read buffer
// Msg& msg  - the metamessage
// Fld& fld  - the metafield
// VALUE obj - the object being written
// VALUE val - the field value on obj being written

// these macros are unhygienic, but that's ok since they are
// local to this file.
#define DEF_RV(type)  VALUE rv_##type(ss_t& ss, Msg& msg, Fld& fld, vector<LazyField> *lazy_fields)
#define INFLATE(val)  RTEST(fld.inflate) ? rb_funcall(fld.inflate, ID_CALL, 1, (val)) : (val)

DEF_RV(INT32)    { return (INT2NUM          (r_varint32(ss)));  }
DEF_RV(UINT32)   { return (UINT2NUM         (r_varint32(ss)));  }
DEF_RV(INT64)    { return (LL2NUM           (r_varint64(ss)));  }
DEF_RV(UINT64)   { return (ULL2NUM          (r_varint64(ss)));  }
DEF_RV(SINT32)   { return (INT2NUM (zz_dec32(r_varint32(ss)))); }
DEF_RV(SINT64)   { return (LL2NUM  (zz_dec64(r_varint64(ss)))); }
DEF_RV(SFIXED32) { return (INT2NUM          (r_int32   (ss)));  }
DEF_RV(FIXED32)  { return (UINT2NUM         (r_int32   (ss)));  }
DEF_RV(SFIXED64) { return (LL2NUM           (r_int64   (ss)));  }
DEF_RV(FIXED64)  { return (ULL2NUM          (r_int64   (ss)));  }

DEF_RV(ENUM) {
  VALUE val = INT2NUM(r_varint32(ss)); 
  if (msg.model->validate_on_read)
    validate_enum(msg, fld, val);
  return val;
}

DEF_RV(FLOAT) {
  uint32_t v = r_int32(ss);
  return (DBL2NUM((double)REINTERPRET(float, v)));
}

DEF_RV(DOUBLE) {
  uint64_t v = r_int64(ss);
  return (DBL2NUM(REINTERPRET(double, v)));
}

DEF_RV(BOOL) {
  uint32_t v = r_varint32(ss);
  return (v == 0 ? Qfalse : Qtrue);
}

DEF_RV(STRING) {
  int32_t len = r_varint32(ss);
  VALUE rstr = rb_str_new(ss_read_chars(ss, len), len);
  return rb_funcall(rstr, FORCE_ID_ENCODING, 1, UTF_8_ENCODING);
}

DEF_RV(BYTES) {
  int32_t len = r_varint32(ss);
  if (fld.get_lazy_type == Qnil) {
    return rb_str_new(ss_read_chars(ss, len), len);
  } else {
    LazyField lf(ss_substream(ss, len), fld);
    lazy_fields->push_back(lf);
    return Qnil;
  }
}

DEF_RV(MESSAGE) {
  int32_t len = r_varint32(ss);
  ss_t tmp_ss = ss_substream(ss, len);
  Msg& embedded_msg = *fld.embedded_msg;
  return embedded_msg.read(tmp_ss, embedded_msg);
}

read_val_func get_val_reader(fld_t fld_type) {
  switch (fld_type) {
    TYPE_MAP(rv);
    default:
      rb_raise(rb_eStandardError, "I don\'t know how to read field type %d", fld_type);
  }
}

VALUE read_obj(ss_t& ss, Msg& msg) {
  VALUE obj = rb_funcall(msg.target, ID_CTOR, 0);
  
  for (Fld& fld : msg.flds_to_enumerate)
    if (fld.label == LABEL_REPEATED)
      set_value(msg, fld, obj, rb_ary_new());

  int num_required_fields_read = 0;

  vector<LazyField> lazy_fields;

  while (ss_more(ss)) {
    uint32_t h = r_varint32(ss);
    fld_num_t fld_num = h >> 3;
    Fld *fld_ptr = msg.find_fld(msg, fld_num);

    if (fld_ptr == NULL) {
      //cerr << "skipping unrecognized field " << msg.name << "." << fld_num << endl;
      wire_t wire_type = h & 7;
      skip(ss, wire_type);
      continue;
    }

    Fld& fld = *fld_ptr;

    if (fld.label != LABEL_REPEATED) {
      if (fld.label == LABEL_REQUIRED)
        num_required_fields_read++;
      set_value(msg, fld, obj, INFLATE(fld.read(ss, msg, fld, &lazy_fields)));
    } else {
      VALUE rb_arr = get_value(msg, fld, obj);
      if (fld.is_packed) {
        uint32_t byte_len = r_varint32(ss);
        ss_t tmp_ss = ss_substream(ss, byte_len);
        while (ss_more(tmp_ss))
          rb_ary_push(rb_arr, INFLATE(fld.read(tmp_ss, msg, fld, NULL)));
      } else {
        VALUE val = fld.read(ss, msg, fld, NULL);
        rb_ary_push(rb_arr, INFLATE(val));
      }
    }
  }

  if (msg.model->validate_on_read && num_required_fields_read < msg.num_required_fields) {
    rb_raise(VALIDATION_ERROR, "Some required fields were missing when reading a %s:\n%s",
             msg.name.c_str(), pp(obj));
  }

  for (LazyField& lf : lazy_fields) {
    Fld& fld = lf.fld;
    Msg& lazy_msg = get_lazy_msg_type(msg, fld, obj);
    VALUE val = lazy_msg.read(lf.ss, lazy_msg);
    set_value(msg, fld, obj, val);
  }

  return obj;
}

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

 
