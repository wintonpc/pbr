#ifndef read_h__
#define read_h__

#include "types.h"
#include "model.h"

read_fld_func get_fld_reader(wire_t wire_type, fld_t fld_type);
read_key_func get_key_reader(wire_t wire_type, fld_t fld_type);

#define DECL_RF(type)  void rf_##type(ss_t& ss, VALUE obj, ID target_field_setter);

DECL_RF(string)
DECL_RF(int32)
DECL_RF(uint32)
DECL_RF(int64)
DECL_RF(uint64)

uint32_t r_var_uint32(ss_t& ss);
uint64_t r_var_uint64(ss_t& ss);

#endif
