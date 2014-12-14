#include <stdio.h>
#include "pbr_ext.h"
#include "model.h"
#include "encode.h"

#define MODEL(handle) (Model*)NUM2LONG(handle);

inline VALUE rb_get(VALUE receiver, const char* name) {
  return rb_funcall(receiver, rb_intern(name), 0);
}

VALUE create_handle(VALUE self) {
  return LONG2NUM((long int)(new Model()));
}

VALUE destroy_handle(VALUE self, VALUE handle) {
  delete MODEL(handle);
  return Qnil;
}

VALUE register_types(VALUE self, VALUE handle, VALUE types) {

  Model* model = MODEL(handle);

  // push msgs
  int num_types = RARRAY_LEN(types);
  for (int i=0; i<num_types; i++) {
    VALUE type = rb_ary_entry(types, i);
    std::string name(RSTRING_PTR(rb_get(type, "name")));
    printf("saw type %s\n", name.c_str());
    Msg msg = make_msg(name, max_zz_field(rb_get(type, "fields")));
    model->msgs.push_back(msg);
  }

  // fill in fields

  return Qnil;
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
  rb_define_singleton_method(ext, "register_types", (VALUE(*)(ANYARGS))register_types, 2);

}
