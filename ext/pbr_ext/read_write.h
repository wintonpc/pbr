#ifndef read_write_h__
#define read_write_h__

#include "common.h"

// https://anteru.net/2007/12/18/200/
#define REINTERPRET(type, value)  *reinterpret_cast<type*>(&value)

#define MAP_TYPE(prefix, type)  case FLD_##type: return prefix##_##type;

#define TYPE_MAP(prefix)                              \
  MAP_TYPE(prefix, STRING)                            \
  MAP_TYPE(prefix, BYTES)                             \
  MAP_TYPE(prefix, INT32)                             \
  MAP_TYPE(prefix, UINT32)                            \
  MAP_TYPE(prefix, INT64)                             \
  MAP_TYPE(prefix, UINT64)                            \
  MAP_TYPE(prefix, SINT32)                            \
  MAP_TYPE(prefix, SINT64)                            \
  MAP_TYPE(prefix, SFIXED32)                          \
  MAP_TYPE(prefix, FIXED32)                           \
  MAP_TYPE(prefix, SFIXED64)                          \
  MAP_TYPE(prefix, FIXED64)                           \
  MAP_TYPE(prefix, FLOAT)                             \
  MAP_TYPE(prefix, DOUBLE)                            \
  MAP_TYPE(prefix, BOOL)                              \
  MAP_TYPE(prefix, MESSAGE)                           \
  MAP_TYPE(prefix, ENUM)                              \
  
inline VALUE get_value(Msg& msg, Fld& fld, VALUE obj) {
  return msg.target_is_hash ?
    rb_funcall(obj, ID_HASH_GET, 1, fld.target_key) :
    rb_funcall(obj, fld.target_field_getter, 0);
}

inline void set_value(Msg& msg, Fld& fld, VALUE obj, VALUE val) {
  msg.target_is_hash ?
    rb_funcall(obj, ID_HASH_SET, 2, fld.target_key, val) :
    rb_funcall(obj, fld.target_field_setter, 1, val);
}

inline void validate_enum(Fld& fld, VALUE val) {
  bool is_valid = false;
  for (VALUE m : fld.enum_values) {
    if (m == val) {
      is_valid = true;
      break;
    }
  }
  
  if (!is_valid)
    rb_raise(VALIDATION_ERROR, "%s.%s is %s, which is not a member of %s",
             fld.msg->name.c_str(), fld.name.c_str(), pp(val), pp(fld.enum_module));
}

#endif
