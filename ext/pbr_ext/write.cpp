#include <iostream>

#include "write.h"

using namespace std;

write_fld_func get_fld_writer(wire_t wire_type, fld_t fld_type) {
  switch (fld_type) {
  case FLD_STRING: return wf_string;
  case FLD_INT32: return wf_int32;
  case FLD_UINT32: return wf_uint32;
  case FLD_INT64: return wf_int64;
  case FLD_UINT64: return wf_uint64;
  default: return NULL;
  }
}

write_key_func get_key_writer(wire_t wire_type, fld_t fld_type) {
  return NULL;
}

#define FVAL()  rb_funcall(obj, target_field, 0) 
#define DEF_WF(type)  void wf_##type(buf_t& buf, VALUE obj, ID target_field)

void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num) {
  uint32_t h = (fld_num << 3) | wire_type;
  w_var_uint32(buf, h);
}

DEF_WF(string) {
  VALUE v = FVAL();
  const char *s = RSTRING_PTR(v);
  int len = RSTRING_LEN(v);
  w_var_uint32(buf, len);
  buf.insert(buf.end(), s, s + len);
}

DEF_WF(int32) { w_var_uint32(buf, NUM2INT(FVAL())); }
DEF_WF(uint32) { w_var_uint32(buf, NUM2UINT(FVAL())); }
DEF_WF(int64) { w_var_uint64(buf, NUM2LL(FVAL())); }
DEF_WF(uint64) { w_var_uint64(buf, NUM2ULL(FVAL())); }

#define DEF_W_VARINT(bits)                                \
  void w_var_uint##bits(buf_t& buf, uint##bits##_t n) {   \
    while (n > 127) {                                     \
      buf.push_back((uint##bits##_t)((n & 127) | 128));   \
      n >>= 7;                                            \
    }                                                     \
    buf.push_back((uint##bits##_t)(n & 127));             \
  }

DEF_W_VARINT(32)
DEF_W_VARINT(64)
