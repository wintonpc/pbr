#ifndef write_h__
#define write_h__

#include "types.h"
#include "model.h"

write_fld_func get_fld_writer(wire_t wire_type, fld_t fld_type);
write_fld_func get_key_writer(wire_t wire_type, fld_t fld_type);

VALUE write_obj(Msg* msg, buf_t& buf, VALUE obj);
void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num);

#endif
