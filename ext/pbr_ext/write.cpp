#include <iostream>

#include "write.h"

using namespace std;

write_fld_func get_fld_writer(wire_t wire_type, fld_t fld_type) {
  switch (fld_type) {
  case FLD_STRING: return wf_string;
  default: return NULL;
  }
}

write_key_func get_key_writer(wire_t wire_type, fld_t fld_type) {
  return NULL;
}

void wf_string(buf_t& buf, VALUE obj, ID target_field) {
  cout << "wf_string" << endl;
  VALUE v = rb_funcall(obj, target_field, 0);
  const char *s = RSTRING_PTR(v);
  int len = RSTRING_LEN(v);
  w_var_uint32(buf, len);
  buf.insert(buf.end(), s, s + len);
}

void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num) {
  cout << "write_header " << wire_type << " " << fld_num << endl;
  uint32_t h = (fld_num << 3) | wire_type;
  w_var_uint32(buf, h);
}

void w_var_uint32(buf_t& buf, uint32_t n) {
  cout << "w_var_uint32 " << n << endl;
  while (n > 127) {
    buf.push_back((uint32_t)((n & 127) | 128));
    n >>= 7;
  }
  buf.push_back((uint32_t)(n & 127));
}
