#ifndef read_write_h__
#define read_write_h__

#define MAP_TYPE(prefix, type)  case FLD_##type: return prefix##_##type;

#define TYPE_MAP(prefix)                            \
  MAP_TYPE(prefix, STRING)                          \
  MAP_TYPE(prefix, INT32)                           \
  MAP_TYPE(prefix, UINT32)                          \
  MAP_TYPE(prefix, INT64)                           \
  MAP_TYPE(prefix, UINT64)                          \
  MAP_TYPE(prefix, SINT32)

#endif