#include <iostream>

#include "common.h"
#include "write.h"
#include "read_write.h"
#include "encode.h"

using namespace std;

// these macros are unhygienic, but that's ok since they are
// local to this file.
#define DEF_WF(type)  void wf_##type(buf_t& buf, VALUE val, Fld& fld)

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
  if (TYPE(v_in) != T_STRING)
    rb_raise(VALIDATION_ERROR, "While writing %s.%s, expected a string but got: %s",
             fld.msg->name.c_str(), fld.name.c_str(), pp(v_in));

  bool is_already_utf8 = rb_funcall(v_in, ID_ENCODING, 0) == UTF_8_ENCODING;
  VALUE v_out = is_already_utf8 ? v_in : rb_funcall(v_in, ID_ENCODE, 1, UTF_8_ENCODING);
  write_bytes(buf, v_out);
}

void pad(buf_t& buf, int num_bytes) {
  for (int i=0; i<num_bytes; i++)
    buf.push_back(0);
}

DEF_WF(MESSAGE) {
  Msg* embedded_msg = fld.embedded_msg;
  uint32_t len_offset = buf.size();
  int32_t estimated_varint_size = embedded_msg->last_varint_size;
  pad(buf, estimated_varint_size);
  uint32_t msg_offset = buf.size();
  embedded_msg->write(*embedded_msg, buf, val);
  uint32_t msg_len = buf.size() - msg_offset;
  int32_t actual_varint_size = varint32_size(msg_len);
  int32_t extra_bytes_needed = actual_varint_size - estimated_varint_size;

  if (extra_bytes_needed > 0) {
    buf.insert(buf.begin() + msg_offset, extra_bytes_needed, 0);
    w_varint32_bytes(buf, len_offset, msg_len, actual_varint_size);
  } else {
    w_varint32_bytes(buf, len_offset, msg_len, estimated_varint_size);    
  }
}

void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num) {
  uint32_t h = (fld_num << 3) | wire_type;
  w_varint32(buf, h);
}

write_val_func get_val_writer(fld_t fld_type) {
  switch (fld_type) {
    TYPE_MAP(wf);
  default:
    rb_raise(rb_eStandardError, "I don\'t know how to write field type %d", fld_type);
  }
}

void write_value(buf_t& buf, Fld& fld, VALUE obj) {
  write_header(buf, fld.wire_type, fld.num);
  fld.write(buf, obj, fld);
}

#define DEFLATE(val)  RTEST(fld.deflate) ? rb_funcall(fld.deflate, ID_CALL, 1, (val)) : (val)

void write_repeated(buf_t& buf, Fld& fld, VALUE arr) {
  int len = RARRAY_LEN(arr);
  for (int i=0; i<len; i++) {
    VALUE elem = DEFLATE(rb_ary_entry(arr, i));
    write_value(buf, fld, elem);
  }        
}

void write_packed(buf_t& buf, Fld& fld, VALUE arr) {
  int len = RARRAY_LEN(arr);
  if (len == 0)
    return;

  write_header(buf, WIRE_LENGTH_DELIMITED, fld.num);
  int32_t len_offset = buf.size();
  pad(buf, MAX_VARINT32_BYTE_SIZE);
  int32_t data_offset = buf.size();
  for (int i=0; i<len; i++) {
    VALUE elem = DEFLATE(rb_ary_entry(arr, i));
    fld.write(buf, elem, fld);
  }
  int32_t data_len = buf.size() - data_offset;
  w_varint32_bytes(buf, len_offset, data_len, MAX_VARINT32_BYTE_SIZE);
}

void write_obj(Msg& msg, buf_t& buf, VALUE obj) {
  int32_t initial_offset = buf.size();
  int num_flds = msg.flds_to_enumerate.size();
  for (int i=0; i<num_flds; i++) {
    Fld& fld = msg.flds_to_enumerate[i];
    VALUE val = get_value(msg, fld, obj);
    if (fld.label != LABEL_REPEATED) {
      if (val == Qnil) {
        if (msg.model->validate_on_write && fld.label == LABEL_REQUIRED)
          rb_raise(VALIDATION_ERROR, "%s.%s is a required field but has value nil. On:\n%s",
                   msg.name.c_str(), fld.name.c_str(), pp(obj));
        else
          continue;
      }
      write_value(buf, fld, DEFLATE(val));
    }
    else {
      if (val == Qnil)
        continue;
      
      if (fld.is_packed)
        write_packed(buf, fld, val);
      else
        write_repeated(buf, fld, val);
    }
  }
  int32_t final_offset = buf.size();
  msg.last_varint_size = varint32_size(final_offset - initial_offset);
}

