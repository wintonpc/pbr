#ifndef write_h__
#define write_h__

#include "types.h"
#include "model.h"

write_fld_func get_fld_writer(wire_t wire_type, fld_t fld_type);
write_key_func get_key_writer(wire_t wire_type, fld_t fld_type);

#define DECL_WF(type)  void wf_##type(buf_t& buf, VALUE obj, ID target_field);

DECL_WF(string)
DECL_WF(int32)
DECL_WF(uint32)
DECL_WF(int64)
DECL_WF(uint64)

void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num);
void w_var_uint32(buf_t& buf, uint32_t n);
void w_var_uint64(buf_t& buf, uint64_t n);

#endif
