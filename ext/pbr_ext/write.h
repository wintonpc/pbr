#ifndef write_h__
#define write_h__

#include "types.h"
#include "model.h"

write_val_func get_val_writer(wire_t wire_type, fld_t fld_type);
void write_obj(buf_t& buf, Msg& msg, VALUE obj);

#endif
