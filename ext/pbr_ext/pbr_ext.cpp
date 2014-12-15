#include <stdio.h>
#include "pbr_ext.h"
#include "model.h"
#include "encode.h"
#include <iostream>

using namespace std;

#define MODEL(handle) (Model*)NUM2LONG(handle);

VALUE rb_get(VALUE receiver, const char* name) {
  return rb_funcall(receiver, rb_intern(name), 0);
}

VALUE rb_call1(VALUE proc, VALUE arg) {
  return rb_funcall(proc, rb_intern("call"), 1);
}

VALUE create_handle(VALUE self) {
  return LONG2NUM((long int)(new Model()));
}

VALUE destroy_handle(VALUE self, VALUE handle) {
  delete MODEL(handle);
  return Qnil;
}

VALUE register_types(VALUE self, VALUE handle, VALUE types, VALUE rule) {

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
    model->msgs.push_back(msg);
  }

  // fill in fields
  for (VALUE type : new_types)
    register_fields(get_msg_for_type(model, type), type);

  // print debugging info
  cout << endl;
  cout << "REGISTERED:" << endl;
  for (Msg& msg : model->msgs) {
    cout << msg.name << endl;
    for (Fld& fld : msg.flds) {
      if (fld.name != "")
        cout << "  " << fld.num << " " << fld.name << " (" << (int)fld.fld_type << ") ["
             << (int)fld.wire_type << "]" << endl;
    }
  }
  cout << "---------------" << endl;
  
  return Qnil;
}

void register_fields(Msg *msg, VALUE type) {
  for (VALUE rFld : arr2vec(rb_get(type, "fields"))) {
    Fld fld;
    fld.num = NUM2INT(rb_get(rFld, "num"));
    fld.name = RSTRING_PTR(sym_to_s(rb_get(rFld, "name")));
    fld.fld_type = NUM2INT(rb_get(rFld, "type"));
    fld.wire_type = wire_type_for_fld_type(fld.fld_type);
    msg->add_fld(msg, fld);
  }
}

Msg* get_msg_for_type(Model* model, VALUE type) {
  return get_msg_by_name(model, type_name(type));
}

string type_name(VALUE type) {
  return string(RSTRING_PTR(rb_get(type, "name")));
}

vector<VALUE> arr2vec(VALUE array) {
  vector<VALUE> v;
  int len = RARRAY_LEN(array);
  for (int i=0; i<len; i++)
    v.push_back(rb_ary_entry(array, i));
  return v;
}

VALUE write(VALUE self, VALUE handle, VALUE obj, VALUE type_name) {
  // Model* model = MODEL(handle);
  // Msg* msg = get_msg_by_name(model, std::string(RSTRING_PTR(type_name)));
  // int num_flds = msg->flds.size();
  // buf_t buf;
  // for (int i=0; i<num_flds; i++) {
  //   Fld* fld = &msg->flds[i];
    
  // }
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

VALUE sym_to_s(VALUE x) {
  if (TYPE(x) == T_SYMBOL)
    return rb_sym_to_s(x);
  else
    return x;
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

}
