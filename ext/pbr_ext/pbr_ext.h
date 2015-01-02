#ifndef pbr_ext_h__
#define pbr_ext_h__

#include <ruby.h>
#include <string>
#include <vector>
#include <map>

#include "common.h"
#include "model.h"


VALUE create_handle(VALUE self, VALUE opts);
VALUE destroy_handle(VALUE self, VALUE handle);
VALUE register_types(VALUE self, VALUE handle, VALUE types, VALUE mapping);
VALUE write(VALUE self, VALUE handle, VALUE obj, VALUE type);
VALUE read(VALUE self, VALUE handle, VALUE sbuf, VALUE type);
void register_fields(Model& model, Msg& msg, VALUE type, VALUE mapping);

#endif

