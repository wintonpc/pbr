#ifndef read_h__
#define read_h__

#include "types.h"
#include "model.h"

read_val_func get_val_reader(wire_t wire_type, fld_t fld_type);
VALUE read_obj(ss_t& ss, Msg& msg);

#endif
