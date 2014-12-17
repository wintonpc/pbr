#ifndef write_h__
#define write_h__

#include "types.h"
#include "model.h"

write_fld_func get_fld_writer(wire_t wire_type, fld_t fld_type);
write_key_func get_key_writer(wire_t wire_type, fld_t fld_type);

void wf_string(buf_t& buf, VALUE obj, ID target_field);
void wf_int32(buf_t& buf, VALUE obj, ID target_field);
void wf_uint32(buf_t& buf, VALUE obj, ID target_field);

void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num);
void w_var_uint32(buf_t& buf, uint32_t n);

#endif
