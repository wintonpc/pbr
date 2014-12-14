#ifndef pbr_ext_h__
#define pbr_ext_h__

#include <ruby.h>
#include <string>
#include <vector>
#include <map>

#include "common.h"
#include "model.h"

VALUE create_handle(VALUE self);
VALUE destroy_handle(VALUE self, VALUE handle);
VALUE register_types(VALUE self, VALUE handle, VALUE types);
void register_fields(Msg& msg, VALUE type);
zz_t max_zz_field(VALUE fields);
VALUE sym_to_s(VALUE x);

#endif

