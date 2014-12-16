#include <stdio.h>
#include "pbr_ext.h"
#include "model.h"
#include "encode.h"
#include <cstdint>
#include <iostream>

using namespace std;

#define MODEL(handle) (Model*)NUM2LONG(handle);

VALUE rb_get(VALUE receiver, const char* name) {
  return rb_funcall(receiver, rb_intern(name), 0);
}

VALUE rb_call1(VALUE proc, VALUE arg) {
  return rb_funcall(proc, rb_intern("call"), 1, arg);
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
    msg.target_is_hash = RTEST(rb_funcall(msg.target, rb_intern("is_a?"), 1, rb_cHash));
    msg.write = msg.target_is_hash ? write_hash : write_obj;
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
    } else {
      fld.target_field = rb_intern(RSTRING_PTR(rb_call1(rb_get(rule, "get_target_field"), rFldName)));
      fld.write_fld = get_fld_writer(fld.wire_type, fld.fld_type);
    }
    msg->add_fld(msg, fld);
  }
}

Msg* get_msg_for_type(Model* model, VALUE type) {
  return get_msg_by_name(model, type_name(type));
}

string type_name(VALUE type) {
  return RSTRING_PTR(rb_get(type, "name"));
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
  write_obj_func write = msg->target_is_hash ? write_hash : write_obj;
  return write(msg, num_flds, buf, obj);
}

VALUE read(VALUE self, VALUE handle, VALUE sbuf, VALUE type) {
  Model* model = MODEL(handle);
  Msg* msg = get_msg_for_type(model, type);
  ss_t ss = ss_make(RSTRING_PTR(sbuf), RSTRING_LEN(sbuf));
  read_obj_func read = msg->target_is_hash ? read_hash : read_obj;
  return read(msg, ss);
}

VALUE read_obj(Msg* msg, ss_t& ss) {
  while (ss_more(ss)) {
    uint8_t b = ss_read_byte(ss);
    cout << "read " << b << endl;
  }
  return Qnil;
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

void w_var_uint32(buf_t& buf, uint32_t n) {
  cout << "w_var_uint32 " << n << endl;
  while (n > 127) {
    buf.push_back((uint32_t)((n & 127) | 128));
    n >>= 7;
  }
  buf.push_back((uint32_t)(n & 127));
}

void write_header(buf_t& buf, wire_t wire_type, fld_num_t fld_num) {
  cout << "write_header " << wire_type << " " << fld_num << endl;
  uint32_t h = (fld_num << 3) | wire_type;
  w_var_uint32(buf, h);
}

void wf_string(buf_t& buf, VALUE obj, ID target_field) {
  cout << "wf_string" << endl;
  VALUE v = rb_funcall(obj, target_field, 0);
  const char *s = RSTRING_PTR(v);
  int len = RSTRING_LEN(v);
  w_var_uint32(buf, len);
  buf.insert(buf.end(), s, s + len);
}

write_fld_func get_fld_writer(wire_t wire_type, fld_t fld_type) {
  switch (fld_type) {
  case FLD_STRING: return wf_string;
  default: return NULL;
  }
}

write_key_func get_key_writer(wire_t wire_type, fld_t fld_type) {
  return NULL;
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
  rb_define_singleton_method(ext, "read", (VALUE(*)(ANYARGS))read, 3);

}
