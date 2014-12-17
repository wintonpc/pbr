#include <iostream>

#include "write.h"
#include "read_write.h"
#include "encode.h"

using namespace std;

#define FVAL()  rb_funcall(obj, target_field, 0) 
#define DEF_WF(type)  void wf_##type(buf_t& buf, VALUE obj, ID target_field)

DEF_WF(STRING) {
  VALUE v = FVAL();
  const char *s = RSTRING_PTR(v);
  int len = RSTRING_LEN(v);
  w_var_uint32(buf, len);
  buf.insert(buf.end(), s, s + len);
}

DEF_WF(INT32) { w_var_uint32(buf, NUM2INT(FVAL())); }
DEF_WF(UINT32) { w_var_uint32(buf, NUM2UINT(FVAL())); }
DEF_WF(INT64) { w_var_uint64(buf, NUM2LL(FVAL())); }
DEF_WF(UINT64) { w_var_uint64(buf, NUM2ULL(FVAL())); }
DEF_WF(SINT32) { w_var_uint32(buf, zz_enc32(NUM2INT(FVAL()))); }
DEF_WF(SINT64) { w_var_uint64(buf, zz_enc64(NUM2LL(FVAL()))); }

void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num) {
  uint32_t h = (fld_num << 3) | wire_type;
  w_var_uint32(buf, h);
}

write_fld_func get_fld_writer(wire_t wire_type, fld_t fld_type) {
  switch (fld_type) { TYPE_MAP(wf); default: return NULL; }
}

write_key_func get_key_writer(wire_t wire_type, fld_t fld_type) {
//switch (fld_type) { TYPE_MAP(wk); default: return NULL; }
return NULL;
}
