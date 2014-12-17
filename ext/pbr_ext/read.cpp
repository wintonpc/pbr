#include <iostream>

#include "read.h"

using namespace std;

read_fld_func get_fld_reader(wire_t wire_type, fld_t fld_type) {
  switch (fld_type) {
  case FLD_STRING: return rf_string;
  default: return NULL;
  }
}

read_key_func get_key_reader(wire_t wire_type, fld_t fld_type) {
  return NULL;
}

void rf_string(ss_t& ss, VALUE obj, ID target_field_setter) {
  //cout << "rf_string" << endl;
  int32_t len = r_var_uint32(ss);
  //cout << "len = " << len << endl;
  VALUE rstr = rb_str_new(ss_read_chars(ss, len), len);
  //cout << inspect(rstr) << endl;
  //cout << inspect(obj) << "." << inspect(ID2SYM(target_field_setter)) << " " << inspect(rstr) << endl;
  rb_funcall(obj, target_field_setter, 1, rstr);
  //cout << "set it" << endl;
}

uint32_t r_var_uint32(ss_t& ss) {
  uint32_t val = 0;
  int sh_amt = 0;

  int iter = 0;

  while (ss_more(ss)) {
    uint8_t b = ss_read_byte(ss);
    cout << "r_var_uint32(" << iter << ") b = " << (int)b << endl;
    val |= (((uint32_t)b) & 127) << sh_amt;

    if ((b & 128) == 0)
      break;
    else
      sh_amt += 7;
    iter++;
  }
  return val;
}
