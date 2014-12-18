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
#define FVAL()  rb_funcall(obj, target_field, 0)
#define DEF_WF(type)  void wf_##type(buf_t& buf, VALUE obj, ID target_field)

DEF_WF(INT32)    { w_varint32(buf,          NUM2INT( FVAL()));  }
DEF_WF(UINT32)   { w_varint32(buf,          NUM2UINT(FVAL()));  }
DEF_WF(INT64)    { w_varint64(buf,          NUM2LL(  FVAL()));  }
DEF_WF(UINT64)   { w_varint64(buf,          NUM2ULL( FVAL()));  }
DEF_WF(SINT32)   { w_varint32(buf, zz_enc32(NUM2INT( FVAL()))); }
DEF_WF(SINT64)   { w_varint64(buf, zz_enc64(NUM2LL(  FVAL()))); }
DEF_WF(SFIXED32) { w_int32(   buf,          NUM2INT( FVAL()));  }
DEF_WF(FIXED32)  { w_int32(   buf,          NUM2UINT(FVAL()));  }
DEF_WF(SFIXED64) { w_int64(   buf,          NUM2LL(  FVAL()));  }
DEF_WF(FIXED64)  { w_int64(   buf,          NUM2ULL( FVAL()));  }

DEF_WF(FLOAT) {
  float v = (float)NUM2DBL(FVAL());
  w_int32(buf, REINTERPRET(uint32_t, v));
}

DEF_WF(DOUBLE) {
  double v = NUM2DBL(FVAL());
  w_int64(buf, REINTERPRET(uint64_t, v));
}

DEF_WF(BOOL) {
  VALUE v = FVAL();
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

DEF_WF(BYTES) { write_bytes(buf, FVAL()); }

DEF_WF(STRING) {
  VALUE v_in = FVAL();
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

void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num) {
  uint32_t h = (fld_num << 3) | wire_type;
  w_varint32(buf, h);
}

write_fld_func get_fld_writer(wire_t wire_type, fld_t fld_type) {
  switch (fld_type) { TYPE_MAP(wf); default: return NULL; }
}

write_key_func get_key_writer(wire_t wire_type, fld_t fld_type) {
  //switch (fld_type) { TYPE_MAP(wk); default: return NULL; }
  return NULL;
}
