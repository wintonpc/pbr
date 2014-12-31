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
ID ID_HASH_GET;
ID ID_HASH_SET;
VALUE VALIDATION_ERROR;

#define MODEL_PTR(handle) (Model*)NUM2LONG(handle)
#define MODEL(handle) *(MODEL_PTR(handle))

VALUE create_handle(VALUE self, VALUE opts) {
  Model* m = new Model();
  m->validate_on_write = rb_hash_get_sym(opts, "validate_on_write");
  m->validate_on_read = rb_hash_get_sym(opts, "validate_on_read");
  return LONG2NUM((long int)(m));
}

VALUE destroy_handle(VALUE self, VALUE handle) {
  delete MODEL_PTR(handle);
  return Qnil;
}

VALUE register_types(VALUE self, VALUE handle, VALUE types, VALUE mapping) {

  Model& model = MODEL(handle);

  // filter out already registered
  vector<VALUE> new_types;
  for (VALUE type : arr2vec(types)) {
    Msg *existing = find_msg_for_type(model, type);
    if (existing == NULL)
      new_types.push_back(type);
  }
  
  // push msgs
  for (VALUE type : new_types) {
    Msg msg = make_msg(type_name(type), max_field_num(rb_get(type, "fields")));
    msg.model = &model;
    msg.target = rb_call1(rb_get(mapping, "get_target_type"), type);
    msg.target_is_hash = RTEST(rb_funcall(msg.target, rb_intern("<="), 1, rb_cHash));
    msg.write = write_obj; // was necessary for indirection at one point. leave as is for
    msg.read = read_obj;   // future flexibility. no noticeable performance hit.
    msg.index = model.msgs.size();
    msg.num_required_fields = 0;
    model.msgs.push_back(msg);
  }

  // fill in fields
  for (VALUE type : new_types)
    register_fields(model, get_msg_for_type(model, type), type, mapping);

  // print debugging info
  cerr << endl;
  cerr << "REGISTERED:" << endl;
  for (Msg& msg : model.msgs) {
    cerr << msg.name << " => " << RSTRING_PTR(rb_inspect(msg.target)) << endl;
    for (Fld& fld : msg.flds_to_enumerate) {
      cerr << "  " << fld.num << " " << fld.name;
      if (msg.target_is_hash)
        cerr << " => " << inspect(fld.target_key);
      else
        cerr << " => " << RSTRING_PTR(rb_inspect(ID2SYM(fld.target_field_getter)))
             << " " << RSTRING_PTR(rb_inspect(ID2SYM(fld.target_field_setter)));
      cerr << " (" << (int)fld.fld_type << ")"
           << " [" << (int)fld.wire_type << "]"
           << "  packed=" << fld.is_packed << endl;
      if (fld.inflate != Qnil)
        cerr  << "      inflate: " << inspect(fld.inflate) << endl;
      if (fld.deflate != Qnil)
        cerr  << "      deflate: " << inspect(fld.deflate) << endl;
    }
  }
  cerr << "---------------" << endl;
  
  return Qnil;
}

#define rb_sym_to_cstr(sym)  RSTRING_PTR(rb_sym_to_s(sym))

void register_fields(Model& model, Msg& msg, VALUE type, VALUE mapping) {
  VALUE deflators = rb_get(type, "deflators");
  VALUE inflators = rb_get(type, "inflators");

  for (VALUE rb_fld : arr2vec(rb_get(type, "fields"))) {
    VALUE fld_name = rb_get(rb_fld, "name");
    Fld fld;
    fld.msg = &msg;
    fld.num = NUM2INT(rb_get(rb_fld, "num"));
    fld.name = rb_sym_to_cstr(fld_name);
    int fld_type = NUM2INT(rb_get(rb_fld, "type"));
    fld.fld_type = fld_type;
    if (fld_type == FLD_MESSAGE)
      fld.embedded_msg = &get_msg_for_type(model, rb_get(rb_fld, "msg_class"));
    fld.wire_type = wire_type_for_fld_type(fld_type);
    fld.label = NUM2INT(rb_get(rb_fld, "label"));
    if (fld.label == LABEL_REQUIRED)
      msg.num_required_fields++;
    fld.is_packed = RTEST(rb_get(rb_fld, "packed"));
    fld.deflate = rb_hash_aref(deflators, fld_name);
    fld.inflate = rb_hash_aref(inflators, fld_name);


    if (msg.target_is_hash) {
      fld.target_key = rb_call1(rb_get(mapping, "get_target_key"), rb_fld);
    } else {
      VALUE tf = rb_call1(rb_get(mapping, "get_target_field"), rb_fld);
      string target_field_name = TYPE(tf) == T_STRING ? RSTRING_PTR(tf) : rb_sym_to_cstr(tf);
      fld.target_field_getter = rb_intern(target_field_name.c_str());
      fld.target_field_setter = rb_intern((target_field_name + "=").c_str());
    }

    fld.write = get_val_writer(fld_type);
    fld.read = get_val_reader(fld_type);
    msg.add_fld(msg, fld);
  }
}

Msg* find_msg_for_type(Model& model, VALUE type) {
  return find_msg_by_name(model, type_name(type));
}

Msg& get_msg_for_type(Model& model, VALUE type) {
  string name = type_name(type);
  Msg* msg = find_msg_by_name(model, name);
  if (msg == NULL)
    rb_raise(rb_eStandardError, "Failed to lookup message named %s", name.c_str());
  return *msg;
}

vector<VALUE> arr2vec(VALUE array) {
  vector<VALUE> v;
  int len = RARRAY_LEN(array);
  for (int i=0; i<len; i++)
    v.push_back(rb_ary_entry(array, i));
  return v;
}

VALUE write(VALUE self, VALUE handle, VALUE obj, VALUE type) {
  cerr << "writing" << endl;
  Model& model = MODEL(handle);
  Msg& msg = get_msg_for_type(model, type);
  buf_t buf;
  buf.reserve(MSG_INITIAL_CAPACITY);
  msg.write(msg, buf, obj);
  return rb_str_new((const char*)buf.data(), buf.size());
}

VALUE read(VALUE self, VALUE handle, VALUE sbuf, VALUE type) {
  cerr << "reading" << endl;
  Model& model = MODEL(handle);
  Msg& msg = get_msg_for_type(model, type);
  ss_t ss = ss_make(RSTRING_PTR(sbuf), RSTRING_LEN(sbuf));
  return msg.read(msg, ss);
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
  default: rb_raise(rb_eStandardError, "unexpected field type %d", (int)fld_type);
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
  ID_HASH_GET = rb_intern("[]");
  ID_HASH_SET = rb_intern("[]=");

  VALUE pbr = rb_define_class("Pbr", rb_cObject);
  VALUE ext = rb_define_module_under(pbr, "Ext");

  VALIDATION_ERROR = rb_const_get(pbr, rb_intern("ValidationError"));

  rb_define_singleton_method(ext, "create_handle", (VALUE(*)(ANYARGS))create_handle, 1);
  rb_define_singleton_method(ext, "destroy_handle", (VALUE(*)(ANYARGS))destroy_handle, 1);
  rb_define_singleton_method(ext, "register_types", (VALUE(*)(ANYARGS))register_types, 3);
  rb_define_singleton_method(ext, "write", (VALUE(*)(ANYARGS))write, 3);
  rb_define_singleton_method(ext, "read", (VALUE(*)(ANYARGS))read, 3);

  rb_require("pp");
}

