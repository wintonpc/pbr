#include <iostream>

#include "write.h"
#include "read_write.h"
#include "encode.h"

using namespace std;

extern ID ID_ENCODING;
extern ID ID_ENCODE;
extern ID ID_CALL;
extern VALUE UTF_8_ENCODING;

// these macros are unhygienic, but that's ok since they are
// local to this file.
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
    cerr << "bad boolean " << inspect(v) << ". wrote false." << endl;
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
    //cerr << "string to write is UTF-8" << endl;
    v = v_in;
  } else {
    //cerr << "string to write is NOT UTF-8" << endl;
    v = rb_funcall(v_in, ID_ENCODE, 1, UTF_8_ENCODING);
  }

  write_bytes(buf, v);
}

DEF_WF(MESSAGE) {
  //cerr << "writing msg field" << endl;
  buf_t tmp_buf;
  Msg* embedded_msg = fld->embedded_msg;
  //cerr << "embedded type: " << embedded_msg->name << endl;
  //cerr << "embedded value: " << inspect(v) << endl;
  embedded_msg->write(embedded_msg, tmp_buf, val);
  w_varint32(buf, tmp_buf.size());
  buf.insert(buf.end(), tmp_buf.begin(), tmp_buf.end());
  //cerr << "wrote embedded message without throwing" << endl;
}

void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num) {
  uint32_t h = (fld_num << 3) | wire_type;
  w_varint32(buf, h);
}

write_val_func get_fld_writer(wire_t wire_type, fld_t fld_type) {
  switch (fld_type) { TYPE_MAP(wf); default: return NULL; }
}

write_val_func get_key_writer(wire_t wire_type, fld_t fld_type) {
  //switch (fld_type) { TYPE_MAP(wk); default: return NULL; }
  return NULL;
}

void write_value(buf_t& buf, Fld* fld, VALUE obj) {
  write_header(buf, fld->wire_type, fld->num);
  fld->write(buf, obj, fld);
}

#define DEFLATE(val)  RTEST(fld->deflate) ? rb_funcall(fld->deflate, ID_CALL, 1, (val)) : (val)

void write_obj(Msg* msg, buf_t& buf, VALUE obj) {
  int num_flds = msg->flds_to_enumerate.size();
  for (int i=0; i<num_flds; i++) {
    Fld* fld = &msg->flds_to_enumerate[i];
    VALUE val = rb_funcall(obj, fld->target_field, 0);
    if (fld->label != LABEL_REPEATED) {
      write_value(buf, fld, DEFLATE(val));
    } else {
      if (val == Qnil)
        continue;
      if (fld->is_packed) {
        int len = RARRAY_LEN(val);
        if (len > 0) {
          write_header(buf, WIRE_LENGTH_DELIMITED, fld->num);
          buf_t tmp_buf;
          for (int i=0; i<len; i++) {
            VALUE elem = DEFLATE(rb_ary_entry(val, i));
            fld->write(tmp_buf, elem, fld);
          }
          w_varint32(buf, tmp_buf.size());
          buf.insert(buf.end(), tmp_buf.begin(), tmp_buf.end());
        }
      } else {
        int len = RARRAY_LEN(val);
        for (int i=0; i<len; i++) {
          VALUE elem = DEFLATE(rb_ary_entry(val, i));
          write_value(buf, fld, elem);
        }        
      }
    }
  }
}

