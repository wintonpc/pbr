#include <stdio.h>
#include "pbr_ext.h"

VALUE create_handle(VALUE self) {
  return INT2NUM(0);
}

void Init_pbr_ext() {
  VALUE pbr, c;

  printf("Init_pbr()\n");

  pbr = rb_define_class("Pbr", rb_cObject);
  c = rb_define_module_under(pbr, "C");

  rb_define_singleton_method(c, "create_handle", create_handle, 0);
  printf("defined Pbr::C::create_handle\n");
}
