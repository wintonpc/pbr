#include <stdio.h>
#include <cstdint>
#include <iostream>

#include "pbr_ext.h"
#include "model.h"
#include "encode.h"
#include "common.h"
#include "ss.h"
#include "types.h"
#include "read.h"
#include "write.h"

using namespace std;

#define MODEL(handle) (Model*)NUM2LONG(handle);

VALUE create_handle(VALUE self) {
  return LONG2NUM((long int)(new Model()));
}

VALUE destroy_handle(VALUE self, VALUE handle) {
  delete MODEL(handle);
  return Qnil;
}

ID ctor_id;

VALUE register_types(VALUE self, VALUE handle, VALUE types, VALUE rule) {

  ctor_id = rb_intern("new");

  Model* model = MODEL(handle);

  // filter out already registered
  vector<VALUE> new_types;
  for (VALUE type : arr2vec(types)) {
    Msg *existing = get_msg_for_type(model, type);
    if (existing == NULL)
      new_types.push_back(type);
  }
  
  // push msgs
  for (VALUE type : new_types) {
    Msg msg = make_msg(type_name(type), max_zz_field(rb_get(type, "fields")));
    msg.target = rb_call1(rb_get(rule, "get_target_type"), type);
    msg.target_is_hash = RTEST(rb_funcall(msg.target, rb_intern("is_a?"), 1, rb_cHash));
    msg.write = msg.target_is_hash ? write_hash : write_obj;
    msg.read = msg.target_is_hash ? read_hash : read_obj;
    model->msgs.push_back(msg);
  }

  // fill in fields
  for (VALUE type : new_types)
    register_fields(get_msg_for_type(model, type), type, rule);

  // print debugging info
  cout << endl;
  cout << "REGISTERED:" << endl;
  for (Msg& msg : model->msgs) {
    cout << msg.name << " => " << RSTRING_PTR(rb_inspect(msg.target)) << endl;
    for (Fld& fld : msg.flds_to_enumerate) {
      cout << "  " << fld.num << " " << fld.name
           << " => " << RSTRING_PTR(rb_inspect(ID2SYM(fld.target_field)))
           << " " << RSTRING_PTR(rb_inspect(ID2SYM(fld.target_field_setter)))
           << " (" << (int)fld.fld_type << ") "
           << "[" << (int)fld.wire_type << "]"
           << endl;
    }
  }
  cout << "---------------" << endl;
  
  return Qnil;
}

void register_fields(Msg *msg, VALUE type, VALUE rule) {
  for (VALUE rFld : arr2vec(rb_get(type, "fields"))) {
    VALUE rFldName = sym_to_s(rb_get(rFld, "name"));
    Fld fld;
    fld.num = NUM2INT(rb_get(rFld, "num"));
    fld.name = RSTRING_PTR(rFldName);
    fld.fld_type = NUM2INT(rb_get(rFld, "type"));
    fld.wire_type = wire_type_for_fld_type(fld.fld_type);
    if (msg->target_is_hash) {
      fld.target_key = rb_call1(rb_get(rule, "get_target_key"), rFldName);
      fld.write_key = get_key_writer(fld.wire_type, fld.fld_type);
      fld.read_key = get_key_reader(fld.wire_type, fld.fld_type);
    } else {
      string target_field_name = RSTRING_PTR(rb_call1(rb_get(rule, "get_target_field"), rFldName));
      fld.target_field = rb_intern(target_field_name.c_str());
      fld.target_field_setter = rb_intern((target_field_name + "=").c_str());
      fld.write_fld = get_fld_writer(fld.wire_type, fld.fld_type);
      fld.read_fld = get_fld_reader(fld.wire_type, fld.fld_type);
    }
    msg->add_fld(msg, fld);
  }
}

Msg* get_msg_for_type(Model* model, VALUE type) {
  return get_msg_by_name(model, type_name(type));
}

vector<VALUE> arr2vec(VALUE array) {
  vector<VALUE> v;
  int len = RARRAY_LEN(array);
  for (int i=0; i<len; i++)
    v.push_back(rb_ary_entry(array, i));
  return v;
}

VALUE write(VALUE self, VALUE handle, VALUE obj, VALUE type) {
  Model* model = MODEL(handle);
  Msg* msg = get_msg_for_type(model, type);
  buf_t buf;
  int num_flds = msg->flds_to_enumerate.size();
  return msg->write(msg, num_flds, buf, obj);
}

VALUE read(VALUE self, VALUE handle, VALUE sbuf, VALUE type) {
  Model* model = MODEL(handle);
  Msg* msg = get_msg_for_type(model, type);
  ss_t ss = ss_make(RSTRING_PTR(sbuf), RSTRING_LEN(sbuf));
  return msg->read(msg, ss);
}

VALUE read_obj(Msg* msg, ss_t& ss) {
  VALUE obj = rb_funcall(msg->target, ctor_id, 0);
  cout << "read_obj " << RSTRING_PTR(rb_inspect(obj)) << " which is a " << RSTRING_PTR(rb_inspect(msg->target)) << endl;
  while (ss_more(ss)) {
    int32_t h = r_var_uint32(ss);
    int32_t wire_type = h & 7;
    int32_t fld_num = h >> 3;
    cout << "wire_type " << wire_type << endl;
    cout << "fld_num " << fld_num << endl;
    Fld* fld = msg->get_fld(msg, fld_num);
    cout << "reading " << fld->name << endl;
    fld->read_fld(ss, obj, fld->target_field_setter);
  }
  return obj;
}
 
VALUE read_hash(Msg* msg, ss_t& ss) {
  return Qnil;
}

VALUE write_obj(Msg* msg, int num_flds, buf_t& buf, VALUE obj) {
  for (int i=0; i<num_flds; i++) {
    Fld* fld = &msg->flds_to_enumerate[i];
    write_header(buf, fld->wire_type, fld->num);
    fld->write_fld(buf, obj, fld->target_field);
  }
  cout << "buf size " << buf.size() << endl;
  cout << "buf[0] " << (int)buf[0] << endl;
  return rb_str_new((const char*)buf.data(), buf.size());
}

VALUE write_hash(Msg* msg, int num_flds, buf_t& buf, VALUE obj) {
  return Qnil;
}

wire_t wire_type_for_fld_type(fld_t fld_type) {
  switch (fld_type) {
  case FLD_STRING:
    return WIRE_LENGTH_DELIMITED;
  default:
    cout << "WARNING: unexpected field type " << (int)fld_type << endl;
    return WIRE_VARINT;
  }
}

zz_t max_zz_field(VALUE flds) {
  zz_t max = 0;
  int len = RARRAY_LEN(flds);
  for (int i=0; i<len; i++) {
    VALUE fld = rb_ary_entry(flds, i);
    zz_t zzfn = zz_enc32(NUM2INT(rb_get(fld, "num")));
    if (zzfn > max)
      max = zzfn;
  }
  return max;
}

extern "C" void Init_pbr_ext() {
  VALUE pbr = rb_define_class("Pbr", rb_cObject);
  VALUE ext = rb_define_module_under(pbr, "Ext");

  rb_define_singleton_method(ext, "create_handle", (VALUE(*)(ANYARGS))create_handle, 0);
  rb_define_singleton_method(ext, "destroy_handle", (VALUE(*)(ANYARGS))destroy_handle, 1);
  rb_define_singleton_method(ext, "register_types", (VALUE(*)(ANYARGS))register_types, 3);
  rb_define_singleton_method(ext, "write", (VALUE(*)(ANYARGS))write, 3);
  rb_define_singleton_method(ext, "read", (VALUE(*)(ANYARGS))read, 3);

}
