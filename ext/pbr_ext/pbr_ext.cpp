#include <stdio.h>
#include "pbr_ext.h"

VALUE create_handle(VALUE self) {
  return LONG2NUM((long int)(new Model()));
}

VALUE destroy_handle(VALUE self, VALUE handle) {
  delete MODEL(handle);
  return Qnil;
}

void add_indexing_fld(Msg& msg, Fld fld) {
  msg.flds[zz_enc32(fld.num)] = fld;
}

const Fld* get_indexing_fld(const Msg& msg, int fld_num) {
  zz_t zz_fld_num = zz_enc32(fld_num);
  if (zz_fld_num >= msg.flds.size())
    return NULL;
  else
    return &msg.flds[zz_fld_num];
}

void add_scanning_fld(Msg& msg, Fld fld) {
  msg.flds.push_back(fld);
}

const Fld* get_scanning_fld(const Msg& msg, int fld_num) {
  int len = msg.flds.size();
  for (int i=0; i<len; i++)
    if (msg.flds[i].num == fld_num)
      return &msg.flds[i];
  return NULL;
}

Msg make_msg(std::string name, zz_t max_zz_fld_num) {
  Msg msg;
  msg.name = name;
  
  if (max_zz_fld_num <= ZZ_FLD_LOOKUP_CUTOFF) {
    msg.flds.reserve(max_zz_fld_num);
    msg.add_fld = add_scanning_fld;
    msg.get_fld = get_scanning_fld;
  }
  else {
    msg.add_fld = add_indexing_fld;
    msg.get_fld = get_indexing_fld;
  }

  return msg;
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

zz_t zz_enc32(int n) {
  return (n << 1) ^ (n >> 31);
}

int zz_dec32(zz_t zz) {
  return (zz >> 1) ^ (-(zz & 1));
}

extern "C" void Init_pbr_ext() {
  VALUE pbr = rb_define_class("Pbr", rb_cObject);
  VALUE ext = rb_define_module_under(pbr, "Ext");

  rb_define_singleton_method(ext, "create_handle", (VALUE(*)(ANYARGS))create_handle, 0);
  rb_define_singleton_method(ext, "destroy_handle", (VALUE(*)(ANYARGS))destroy_handle, 1);
  rb_define_singleton_method(ext, "register_types", (VALUE(*)(ANYARGS))register_types, 2);

}
