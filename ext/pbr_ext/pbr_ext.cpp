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

// "constants" we don't want to compute repeatedly.
// set in Init_pbr_ext.
ID ID_CTOR; 
VALUE UTF_8_ENCODING;
ID ID_ENCODE;
ID ID_ENCODING;
ID FORCE_ID_ENCODING;
ID ID_CALL;

#define MODEL(handle) (Model*)NUM2LONG(handle);

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
    Msg msg = make_msg(type_name(type), max_field_num(rb_get(type, "fields")));
    msg.target = rb_call1(rb_get(rule, "get_target_type"), type);
    msg.target_is_hash = RTEST(rb_funcall(msg.target, rb_intern("is_a?"), 1, rb_cHash));
    msg.write = msg.target_is_hash ? write_hash : write_obj;
    msg.read = msg.target_is_hash ? read_hash : read_obj;
    msg.index = model->msgs.size();
    model->msgs.push_back(msg);
  }

  // fill in fields
  for (VALUE type : new_types)
    register_fields(model, get_msg_for_type(model, type), type, rule);

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
           << "  packed=" << fld.is_packed << endl
           << "  inflate: " << inspect(fld.inflate) << endl
           << "  deflate: " << inspect(fld.deflate) << endl
           << endl;
    }
  }
  cout << "---------------" << endl;
  
  return Qnil;
}

void register_fields(Model* model, Msg *msg, VALUE type, VALUE rule) {
  VALUE deflators = rb_get(type, "deflators");
  VALUE inflators = rb_get(type, "inflators");

  for (VALUE rFld : arr2vec(rb_get(type, "fields"))) {
    VALUE rFldName = sym_to_s(rb_get(rFld, "name"));
    Fld fld;
    fld.num = NUM2INT(rb_get(rFld, "num"));
    fld.name = RSTRING_PTR(rFldName);
    fld.fld_type = NUM2INT(rb_get(rFld, "type"));
    if (fld.fld_type == FLD_MESSAGE)
      fld.embedded_msg = get_msg_for_type(model, rb_get(rFld, "msg_class"));
    fld.wire_type = wire_type_for_fld_type(fld.fld_type);
    fld.label = NUM2INT(rb_get(rFld, "label"));
    fld.is_packed = RTEST(rb_get(rFld, "packed"));
    fld.deflate = rb_hash_aref(deflators, rFldName);
    fld.inflate = rb_hash_aref(inflators, rFldName);

    if (fld.wire_type == -1)
      continue;
    if (msg->target_is_hash) {
      fld.target_key = rb_call1(rb_get(rule, "get_target_key"), rFldName);
      fld.write = get_key_writer(fld.wire_type, fld.fld_type);
      fld.read = get_key_reader(fld.wire_type, fld.fld_type);
    } else {
      string target_field_name = RSTRING_PTR(rb_call1(rb_get(rule, "get_target_field"), rFldName));
      fld.target_field = rb_intern(target_field_name.c_str());
      fld.target_field_setter = rb_intern((target_field_name + "=").c_str());
      fld.write = get_fld_writer(fld.wire_type, fld.fld_type);
      fld.read = get_fld_reader(fld.wire_type, fld.fld_type);
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
  cout << "writing" << endl;
  Model* model = MODEL(handle);
  Msg* msg = get_msg_for_type(model, type);
  buf_t buf;
  return msg->write(msg, buf, obj);
}

VALUE read(VALUE self, VALUE handle, VALUE sbuf, VALUE type) {
  cout << "reading" << endl;
  Model* model = MODEL(handle);
  Msg* msg = get_msg_for_type(model, type);
  ss_t ss = ss_make(RSTRING_PTR(sbuf), RSTRING_LEN(sbuf));
  return msg->read(msg, ss);
}

VALUE read_hash(Msg* msg, ss_t& ss) {
  return Qnil;
}

VALUE write_hash(Msg* msg, buf_t& buf, VALUE obj) {
  return Qnil;
}

wire_t wire_type_for_fld_type(fld_t fld_type) {
  switch (fld_type) {
  case FLD_DOUBLE:   return WIRE_64BIT;
  case FLD_FLOAT:    return WIRE_32BIT;
  case FLD_INT64:    return WIRE_VARINT;
  case FLD_UINT64:   return WIRE_VARINT;
  case FLD_INT32:    return WIRE_VARINT;
  case FLD_FIXED64:  return WIRE_64BIT;
  case FLD_FIXED32:  return WIRE_32BIT;
  case FLD_BOOL:     return WIRE_VARINT;
  case FLD_STRING:   return WIRE_LENGTH_DELIMITED;
  case FLD_MESSAGE:  return WIRE_LENGTH_DELIMITED;
  case FLD_BYTES:    return WIRE_LENGTH_DELIMITED;
  case FLD_UINT32:   return WIRE_VARINT;
  case FLD_ENUM:     return WIRE_VARINT;
  case FLD_SFIXED32: return WIRE_32BIT;
  case FLD_SFIXED64: return WIRE_64BIT;
  case FLD_SINT32:   return WIRE_VARINT;
  case FLD_SINT64:   return WIRE_VARINT;
  default:
    cout << "WARNING: unexpected field type " << (int)fld_type << endl;
    return -1;
  }
}

int32_t max_field_num(VALUE flds) {
  int32_t max = 0;
  int len = RARRAY_LEN(flds);
  for (int i=0; i<len; i++) {
    VALUE fld = rb_ary_entry(flds, i);
    int32_t fn = NUM2INT(rb_get(fld, "num"));
    if (fn > max)
      max = fn;
  }
  return max;
}

extern "C" void Init_pbr_ext() {
  ID_CTOR = rb_intern("new");
  ID_ENCODE = rb_intern("encode");
  ID_ENCODING = rb_intern("encoding");
  FORCE_ID_ENCODING = rb_intern("force_encoding");
  ID_CALL = rb_intern("call");
  VALUE encoding = rb_const_get(rb_cObject, rb_intern("Encoding"));
  UTF_8_ENCODING = rb_const_get(encoding, rb_intern("UTF_8"));

  VALUE pbr = rb_define_class("Pbr", rb_cObject);
  VALUE ext = rb_define_module_under(pbr, "Ext");

  rb_define_singleton_method(ext, "create_handle", (VALUE(*)(ANYARGS))create_handle, 0);
  rb_define_singleton_method(ext, "destroy_handle", (VALUE(*)(ANYARGS))destroy_handle, 1);
  rb_define_singleton_method(ext, "register_types", (VALUE(*)(ANYARGS))register_types, 3);
  rb_define_singleton_method(ext, "write", (VALUE(*)(ANYARGS))write, 3);
  rb_define_singleton_method(ext, "read", (VALUE(*)(ANYARGS))read, 3);
}

