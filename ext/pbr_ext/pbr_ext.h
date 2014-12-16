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
VALUE write_obj(Msg* msg, int num_flds, buf_t& buf, VALUE obj);
VALUE write_hash(Msg* msg, int num_flds, buf_t& buf, VALUE obj);
void register_fields(Msg *msg, VALUE type, VALUE rule);
zz_t max_zz_field(VALUE fields);
VALUE sym_to_s(VALUE x);
wire_t wire_type_for_fld_type(fld_t fld_type);
std::string type_name(VALUE type);
Msg* get_msg_for_type(Model* model, VALUE type);
std::vector<VALUE> arr2vec(VALUE array);
void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num);
write_fld_func get_fld_writer(wire_t wire_type, fld_t fld_type);
write_key_func get_key_writer(wire_t wire_type, fld_t fld_type);

#endif

