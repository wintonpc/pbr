#include <iostream>

#include "write.h"
#include "read_write.h"
#include "encode.h"

using namespace std;

extern ID ID_ENCODING;
extern ID ID_ENCODE;
extern VALUE UTF_8_ENCODING;

// these macros are unhygienic, but that's ok since they are
// local to this file.
//#define FVAL()  rb_funcall(obj, fld->target_field, 0)
#define DEF_WF(type)  void wf_##type(buf_t& buf, VALUE val, Fld* fld)

DEF_WF(INT32)    { w_varint32(buf,          NUM2INT (val));  }
DEF_WF(UINT32)   { w_varint32(buf,          NUM2UINT(val));  }
DEF_WF(INT64)    { w_varint64(buf,          NUM2LL  (val));  }
DEF_WF(UINT64)   { w_varint64(buf,          NUM2ULL (val));  }
DEF_WF(SINT32)   { w_varint32(buf, zz_enc32(NUM2INT (val))); }
DEF_WF(SINT64)   { w_varint64(buf, zz_enc64(NUM2LL  (val))); }
DEF_WF(SFIXED32) { w_int32   (buf,          NUM2INT (val));  }
DEF_WF(FIXED32)  { w_int32   (buf,          NUM2UINT(val));  }
DEF_WF(SFIXED64) { w_int64   (buf,          NUM2LL  (val));  }
DEF_WF(FIXED64)  { w_int64   (buf,          NUM2ULL (val));  }

DEF_WF(ENUM)     { w_varint32(buf,          NUM2INT (val));  }

DEF_WF(FLOAT) {
  float v = (float)NUM2DBL(val);
  w_int32(buf, REINTERPRET(uint32_t, v));
}

DEF_WF(DOUBLE) {
  double v = NUM2DBL(val);
  w_int64(buf, REINTERPRET(uint64_t, v));
}

DEF_WF(BOOL) {
  VALUE v = val;
  if (v == Qtrue)
    w_varint32(buf, 1);
  else if (v == Qfalse)
    w_varint32(buf, 0);
  else {
    w_varint32(buf, 0);
    cout << "bad boolean " << inspect(v) << ". wrote false." << endl;
  }
}

void write_bytes(buf_t& buf, VALUE rstr) {
  const char *s = RSTRING_PTR(rstr);
  int len = RSTRING_LEN(rstr);
  w_varint32(buf, len);
  buf.insert(buf.end(), s, s + len);
}

DEF_WF(BYTES) { write_bytes(buf, val); }

DEF_WF(STRING) {
  VALUE v_in = val;
  VALUE v;

  if (rb_funcall(v_in, ID_ENCODING, 0) == UTF_8_ENCODING) {
    //cout << "string to write is UTF-8" << endl;
    v = v_in;
  } else {
    //cout << "string to write is NOT UTF-8" << endl;
    v = rb_funcall(v_in, ID_ENCODE, 1, UTF_8_ENCODING);
  }

  write_bytes(buf, v);
}

DEF_WF(MESSAGE) {
  cout << "writing msg field" << endl;
  buf_t tmp_buf;
  Msg* embedded_msg = fld->embedded_msg;
  cout << "embedded type: " << embedded_msg->name << endl;
  //cout << "embedded value: " << inspect(v) << endl;
  embedded_msg->write(embedded_msg, tmp_buf, val);
  w_varint32(buf, tmp_buf.size());
  buf.insert(buf.end(), tmp_buf.begin(), tmp_buf.end());
  cout << "wrote embedded message without throwing" << endl;
}

void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num) {
  uint32_t h = (fld_num << 3) | wire_type;
  w_varint32(buf, h);
}

write_fld_func get_fld_writer(wire_t wire_type, fld_t fld_type) {
  switch (fld_type) { TYPE_MAP(wf); default: return NULL; }
}

write_fld_func get_key_writer(wire_t wire_type, fld_t fld_type) {
  //switch (fld_type) { TYPE_MAP(wk); default: return NULL; }
  return NULL;
}
