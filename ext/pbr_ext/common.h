#ifndef common_h__
#define common_h__

#include <cstdint>
#include <vector>
#include <ruby.h>

#include "types.h"

const zz_t ZZ_FLD_LOOKUP_CUTOFF = 1024;

inline const char* inspect(VALUE x) {
  return RSTRING_PTR(rb_inspect(x));
}

inline VALUE rb_get(VALUE receiver, const char* name) {
  return rb_funcall(receiver, rb_intern(name), 0);
}

inline VALUE rb_call1(VALUE proc, VALUE arg) {
  return rb_funcall(proc, rb_intern("call"), 1, arg);
}

inline VALUE sym_to_s(VALUE x) {
  if (TYPE(x) == T_SYMBOL)
    return rb_sym_to_s(x);
  else
    return x;
}

inline std::string type_name(VALUE type) {
  return RSTRING_PTR(rb_get(type, "name"));
}



#endif
