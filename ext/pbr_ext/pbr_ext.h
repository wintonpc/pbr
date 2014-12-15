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
VALUE register_types(VALUE self, VALUE handle, VALUE types, VALUE rule);
VALUE write(VALUE self, VALUE handle, VALUE obj, VALUE type_name);
void register_fields(Msg *msg, VALUE type);
zz_t max_zz_field(VALUE fields);
VALUE sym_to_s(VALUE x);
wire_t wire_type_for_fld_type(fld_t fld_type);
std::string type_name(VALUE type);
Msg* get_msg_for_type(Model* model, VALUE type);
std::vector<VALUE> arr2vec(VALUE array);

#endif

