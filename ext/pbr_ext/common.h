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

extern ID ID_CTOR;
extern VALUE UTF_8_ENCODING;
extern ID ID_ENCODE;
extern ID ID_ENCODING;
extern ID FORCE_ID_ENCODING;
extern ID ID_CALL;
extern ID ID_HASH_GET;
extern ID ID_HASH_SET;
extern VALUE VALIDATION_ERROR;

inline const char* inspect(VALUE x) {
  return RSTRING_PTR(rb_inspect(x));
}

inline const char* pp(VALUE x) {
  return RSTRING_PTR(rb_funcall(rb_const_get(rb_cObject, rb_intern("PP")), rb_intern("pp"), 2, x, rb_str_new("", 0)));
}

inline VALUE rb_get(VALUE receiver, const char* name) {
  return rb_funcall(receiver, rb_intern(name), 0);
}

inline VALUE rb_call1(VALUE proc, VALUE arg) {
  return rb_funcall(proc, rb_intern("call"), 1, arg);
}

inline VALUE rb_hash_get_sym(VALUE hash, const char* sym) {
  return rb_funcall(hash, ID_HASH_GET, 1, ID2SYM(rb_intern(sym)));
}

inline std::string type_name(VALUE type) {
  return RSTRING_PTR(rb_get(type, "type_name"));
}

#endif
