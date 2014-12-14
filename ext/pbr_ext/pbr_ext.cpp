#include <stdio.h>
#include "pbr_ext.h"

VALUE create_handle(VALUE self) {
  return LONG2NUM((long int)(new Model()));
}

VALUE destroy_handle(VALUE self, VALUE handle) {
  delete MODEL(handle);
  return Qnil;
}

const Msg create_msg(std::string name, zz_t max_zz_fld_num) {
  if (max_zz_fld_num <= ZZ_FLD_LOOKUP_CUTOFF)
    return IndexingMsg(name, max_zz_fld_num);
  else
    return ScanningMsg(name);
}

IndexingMsg::IndexingMsg(std::string name, zz_t max_zz_fld_num)
  : name(name) {
  flds.reserve(max_zz_fld_num);
}

ScanningMsg::ScanningMsg(std::string name)
  : name(name) {
}

void IndexingMsg::add_fld(Fld fld) {
  flds[zz_enc32(fld.num)] = fld;
}

Fld* IndexingMsg::get_fld(int fld_num) {
  if (fld_num >= flds.size())
    return NULL;
  else
    return &flds[zz_enc32(fld_num)];
}

void ScanningMsg::add_fld(Fld fld) {
  flds.push_back(fld);
}

Fld* ScanningMsg::get_fld(int fld_num) {
  for (int i=0; i<flds.size(); i++)
    if (flds[i].num == fld_num)
      return &flds[i];
  return NULL;
}



VALUE register_types(VALUE self, VALUE handle, VALUE types) {

  Model* model = MODEL(handle);

  // push msgs
  int num_types = RARRAY_LEN(types);
  for (int i=0; i<num_types; i++) {
    VALUE type = rb_ary_entry(types, i);
    std::string name(RSTRING_PTR(rb_get(type, "name")));
    printf("saw type %s\n", name.c_str());
    Msg msg = Msg::create(name, max_zz_field(rb_get(type, "fields")));
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

zz_t zz_dec32(zz_t zz) {
  return (zz >> 1) ^ (-(zz & 1));
}

extern "C" void Init_pbr_ext() {
  VALUE pbr = rb_define_class("Pbr", rb_cObject);
  VALUE ext = rb_define_module_under(pbr, "Ext");

  rb_define_singleton_method(ext, "create_handle", (VALUE(*)(ANYARGS))create_handle, 0);
  rb_define_singleton_method(ext, "destroy_handle", (VALUE(*)(ANYARGS))destroy_handle, 1);
  rb_define_singleton_method(ext, "register_types", (VALUE(*)(ANYARGS))register_types, 2);

}
