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

void pad(buf_t& buf, int num_bytes) {
  for (int i=0; i<num_bytes; i++)
    buf.push_back(0);
}

DEF_WF(MESSAGE) {
  Msg* embedded_msg = fld->embedded_msg;
  uint32_t len_offset = buf.size();
  int32_t estimated_varint_size = embedded_msg->last_varint_size;
  pad(buf, estimated_varint_size);
  uint32_t msg_offset = buf.size();
  embedded_msg->write(embedded_msg, buf, val);
  uint32_t msg_len = buf.size() - msg_offset;
  int32_t actual_varint_size = varint32_size(msg_len);
  int32_t extra_bytes_needed = actual_varint_size - estimated_varint_size;

  /*
  cerr << "len_offset: " << len_offset << endl;
  cerr << "msg_offset: " << msg_offset << endl;
  cerr << "msg_len: " << msg_len << endl;

  cerr << "estimated_varint_size: " << estimated_varint_size << endl;
  cerr << "actual_varint_size: " << actual_varint_size << endl;
  cerr << "extra_bytes_needed: " << extra_bytes_needed << endl;

  cerr << endl;
  */
  if (extra_bytes_needed > 0) {
    buf.insert(buf.begin() + msg_offset, extra_bytes_needed, 0);
    w_varint32_bytes(buf, len_offset, msg_len, actual_varint_size);
    //cerr << "inserted extra bytes for varint length" << endl;
  } else {
    w_varint32_bytes(buf, len_offset, msg_len, estimated_varint_size);    
  }
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

void write_repeated(buf_t& buf, Fld* fld, VALUE arr) {
  int len = RARRAY_LEN(arr);
  for (int i=0; i<len; i++) {
    VALUE elem = DEFLATE(rb_ary_entry(arr, i));
    write_value(buf, fld, elem);
  }        
}

void write_obj(Msg* msg, buf_t& buf, VALUE obj) {
  int32_t initial_offset = buf.size();
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
          int32_t len_offset = buf.size();
          pad(buf, MAX_VARINT32_BYTE_SIZE);
          int32_t data_offset = buf.size();
          for (int i=0; i<len; i++) {
            VALUE elem = DEFLATE(rb_ary_entry(val, i));
            fld->write(buf, elem, fld);
          }
          int32_t data_len = buf.size() - data_offset;
          w_varint32_bytes(buf, len_offset, data_len, MAX_VARINT32_BYTE_SIZE);
        }
      } else {
        write_repeated(buf, fld, val);
      }
    }
  }
  int32_t final_offset = buf.size();
  msg->last_varint_size = varint32_size(final_offset - initial_offset);
  //cerr << "msg_size: " << (final_offset - initial_offset) << "; varint: " << msg->last_varint_size << endl;
}

