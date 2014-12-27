#ifndef common_h__
#define common_h__

#include <cstdint>
#include <vector>
#include <ruby.h>

#include "types.h"

const int32_t FLD_LOOKUP_CUTOFF = 1024;
const int32_t MSG_INITIAL_CAPACITY = 16384;
const int32_t EMBEDDED_MSG_INITIAL_CAPACITY = 1024;
const int32_t MAX_VARINT32_BYTE_SIZE = 5;

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
  return RSTRING_PTR(rb_get(type, "type_name"));
}

#endif
