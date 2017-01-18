#include <iostream>

#include "common.h"
#include "write.h"
#include "read_write.h"
#include "encode.h"

using namespace std;

#define WRITE_ARGS  buf_t& buf, Msg& msg, Fld& fld, VALUE obj, VALUE val

void write_bytes(buf_t& buf, VALUE rstr);
void pad(buf_t& buf, int num_bytes);
void write_bytes(buf_t& buf, VALUE rstr);
void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num);
void write_value(WRITE_ARGS);
void write_packed(WRITE_ARGS);
void write_repeated(WRITE_ARGS);
void write_embedded(buf_t& buf, Msg& msg, VALUE obj);

// conventional variables used in this file
// buf_t& buf  - the write buffer
// Msg& msg  - the metamessage
// Fld& fld  - the metafield
// VALUE obj - the object being written
// VALUE val - the field value on obj being written

// these macros are unhygienic, but that's ok since they are
// local to this file.
#define DEF_WV(type)  void wv_##type(WRITE_ARGS)
#define DEFLATE(val)  RTEST(fld.deflate) ? rb_funcall(fld.deflate, ID_CALL, 1, (val)) : (val)

// define all the type writers up front to avoid the need for declarations.

DEF_WV(INT32)    { w_varint32(buf,          NUM2INT (val));  }
DEF_WV(UINT32)   { w_varint32(buf,          NUM2UINT(val));  }
DEF_WV(INT64)    { w_varint64(buf,          NUM2LL  (val));  }
DEF_WV(UINT64)   { w_varint64(buf,          NUM2ULL (val));  }
DEF_WV(SINT32)   { w_varint32(buf, zz_enc32(NUM2INT (val))); }
DEF_WV(SINT64)   { w_varint64(buf, zz_enc64(NUM2LL  (val))); }
DEF_WV(SFIXED32) { w_int32   (buf,          NUM2INT (val));  }
DEF_WV(FIXED32)  { w_int32   (buf,          NUM2UINT(val));  }
DEF_WV(SFIXED64) { w_int64   (buf,          NUM2LL  (val));  }
DEF_WV(FIXED64)  { w_int64   (buf,          NUM2ULL (val));  }

DEF_WV(ENUM) {
  if (msg.model->validate_on_write)
    validate_enum(msg, fld, val);
  w_varint32(buf, NUM2INT(val));
}

DEF_WV(FLOAT) {
  float v = (float)NUM2DBL(val);
  w_int32(buf, REINTERPRET(uint32_t, v));
}

DEF_WV(DOUBLE) {
  double v = NUM2DBL(val);
  w_int64(buf, REINTERPRET(uint64_t, v));
}

DEF_WV(BOOL) {
  if (val == Qtrue)
    w_varint32(buf, 1);
  else if (val == Qfalse)
    w_varint32(buf, 0);
  else {
    rb_raise(VALIDATION_ERROR, "%s.%s should be a boolean but is: %s",
             msg.name.c_str(), fld.name.c_str(), pp(val));
  }
}

DEF_WV(BYTES) {
  if (TYPE(val) == T_STRING)
    write_bytes(buf, val);
  else if (fld.get_lazy_type != Qnil)
    write_embedded(buf, get_lazy_msg_type(msg, fld, obj), val);
  else
    rb_raise(VALIDATION_ERROR, "While writing %s.%s, expected a string but got: %s",
             msg.name.c_str(), fld.name.c_str(), pp(val));
}

DEF_WV(STRING) {
  VALUE v_in = val;
  if (TYPE(v_in) != T_STRING)
    rb_raise(VALIDATION_ERROR, "While writing %s.%s, expected a string but got: %s",
             msg.name.c_str(), fld.name.c_str(), pp(v_in));

  bool is_already_utf8 = rb_funcall(v_in, ID_ENCODING, 0) == UTF_8_ENCODING;
  VALUE v_out = is_already_utf8 ? v_in : rb_funcall(v_in, ID_ENCODE, 1, UTF_8_ENCODING);
  write_bytes(buf, v_out);
}

DEF_WV(MESSAGE) {
  Msg& embedded_msg = *fld.embedded_msg;
  write_embedded(buf, embedded_msg, val);
}

write_val_func get_val_writer(fld_t fld_type) {
  switch (fld_type) {
    TYPE_MAP(wv);
  default:
    rb_raise(rb_eStandardError, "I don\'t know how to write field type %d", fld_type);
  }
}

// private

void write_obj(buf_t& buf, Msg& msg, VALUE obj) {
  if (!rb_obj_is_kind_of(obj, msg.target))
    rb_raise(VALIDATION_ERROR, "Expected %s but got: %s", pp(msg.target), pp(obj));

  int32_t initial_offset = buf.size();
  for (Fld& fld : msg.flds_to_enumerate) {
    VALUE val = get_value(msg, fld, obj);
    if (fld.label != LABEL_REPEATED) {
      if (val == Qnil) {
        if (msg.model->validate_on_write && fld.label == LABEL_REQUIRED)
          rb_raise(VALIDATION_ERROR, "%s.%s is a required field but has value nil. On:\n%s",
                   msg.name.c_str(), fld.name.c_str(), pp(obj));
        else
          continue;
      }
      write_value(buf, msg, fld, obj, DEFLATE(val));
    }
    else {
      if (val == Qnil)
        continue;
      else if (TYPE(val) != T_ARRAY)
        rb_raise(VALIDATION_ERROR, "%s.%s: Expected array but got: %s",
                 msg.name.c_str(), fld.name.c_str(), pp(val));
      else if (fld.is_packed)
        write_packed(buf, msg, fld, obj, val);
      else
        write_repeated(buf, msg, fld, obj, val);
    }
  }
  int32_t final_offset = buf.size();
  msg.last_varint_size = varint32_size(final_offset - initial_offset);
}

void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num) {
  uint32_t h = (fld_num << 3) | wire_type;
  w_varint32(buf, h);
}

void write_bytes(buf_t& buf, VALUE rstr) {
  const char *s = RSTRING_PTR(rstr);
  int len = RSTRING_LEN(rstr);
  w_varint32(buf, len);
  buf.insert(buf.end(), s, s + len);
}

void write_value(WRITE_ARGS) {
  write_header(buf, fld.wire_type, fld.num);
  fld.write(buf, msg, fld, obj, val);
}

void write_repeated(WRITE_ARGS) {
  int len = RARRAY_LEN(val);
  for (int i=0; i<len; i++) {
    VALUE elem = DEFLATE(rb_ary_entry(val, i));
    write_value(buf, msg, fld, obj, elem);
  }        
}

void write_packed(WRITE_ARGS) {
  int len = RARRAY_LEN(val);
  if (len == 0)
    return;

  write_header(buf, WIRE_LENGTH_DELIMITED, fld.num);
  int32_t len_offset = buf.size();
  pad(buf, MAX_VARINT32_BYTE_SIZE);
  int32_t data_offset = buf.size();
  for (int i=0; i<len; i++) {
    VALUE elem = DEFLATE(rb_ary_entry(val, i));
    fld.write(buf, msg, fld, obj, elem);
  }
  int32_t data_len = buf.size() - data_offset;
  w_varint32_bytes(buf, len_offset, data_len, MAX_VARINT32_BYTE_SIZE);
}

void write_embedded(buf_t& buf, Msg& msg, VALUE obj) {
  uint32_t len_offset = buf.size();
  int32_t estimated_varint_size = msg.last_varint_size;
  pad(buf, estimated_varint_size);
  uint32_t msg_offset = buf.size();
  msg.write(buf, msg, obj);
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

void pad(buf_t& buf, int num_bytes) {
  for (int i=0; i<num_bytes; i++)
    buf.push_back(0);
}
