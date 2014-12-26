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
VALUE write(VALUE self, VALUE handle, VALUE obj, VALUE type);
void write_hash(Msg* msg, buf_t& buf, VALUE obj);
VALUE read(VALUE self, VALUE handle, VALUE sbuf, VALUE type);
VALUE read_hash(Msg* msg, ss_t& ss);
void register_fields(Model* model, Msg *msg, VALUE type, VALUE rule);
int32_t max_field_num(VALUE fields);
VALUE sym_to_s(VALUE x);
wire_t wire_type_for_fld_type(fld_t fld_type);
std::string type_name(VALUE type);
Msg* get_msg_for_type(Model* model, VALUE type);
std::vector<VALUE> arr2vec(VALUE array);
void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num);
write_val_func get_fld_writer(wire_t wire_type, fld_t fld_type);
write_val_func get_key_writer(wire_t wire_type, fld_t fld_type);
read_val_func get_fld_reader(wire_t wire_type, fld_t fld_type);
read_val_func get_key_reader(wire_t wire_type, fld_t fld_type);

#endif

