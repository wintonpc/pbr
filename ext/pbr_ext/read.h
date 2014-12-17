#ifndef read_h__
#define read_h__

#include "types.h"
#include "model.h"

read_fld_func get_fld_reader(wire_t wire_type, fld_t fld_type);
read_key_func get_key_reader(wire_t wire_type, fld_t fld_type);

void rf_string(ss_t& ss, VALUE obj, ID target_field_setter);
void rf_int32(ss_t& ss, VALUE obj, ID target_field_setter);
void rf_uint32(ss_t& ss, VALUE obj, ID target_field_setter);

uint32_t r_var_uint32(ss_t& ss);

#endif
