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
VALUE register_types(VALUE self, VALUE handle, VALUE types, VALUE mapping);
VALUE write(VALUE self, VALUE handle, VALUE obj, VALUE type);
VALUE read(VALUE self, VALUE handle, VALUE sbuf, VALUE type);
void register_fields(Model& model, Msg& msg, VALUE type, VALUE mapping);
int32_t max_field_num(VALUE fields);
wire_t wire_type_for_fld_type(fld_t fld_type);
std::string type_name(VALUE type);
Msg* find_msg_for_type(Model& model, VALUE type);
Msg& get_msg_for_type(Model& model, VALUE type);
std::vector<VALUE> arr2vec(VALUE array);
void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num);
write_val_func get_val_writer(fld_t fld_type);
read_val_func get_val_reader(fld_t fld_type);

#endif

