#ifndef pbr_ext_h__
#define pbr_ext_h__

#include <ruby.h>
#include <string>
#include <vector>
#include <map>

typedef unsigned char wire_t;
typedef unsigned char fld_t;
typedef unsigned int zz_t;
typedef long unsigned int zz64_t;

const wire_t WIRE_VARINT = 0;
const wire_t WIRE_64BIT = 1;
const wire_t WIRE_LENGTH_DELIMITED = 2;
//const wire_t WIRE_START_GROUP = 3;
//const wire_t WIRE_END_GROUP = 4;
const wire_t WIRE_32BIT = 5;

const fld_t FLD_DOUBLE = 1;
const fld_t FLD_FLOAT = 2;
const fld_t FLD_INT64 = 3;
const fld_t FLD_UINT64 = 4;
const fld_t FLD_INT32 = 5;
const fld_t FLD_FIXED64 = 6;
const fld_t FLD_FIXED32 = 7;
const fld_t FLD_BOOL = 8;
const fld_t FLD_STRING = 9;
const fld_t FLD_GROUP = 10;
const fld_t FLD_MESSAGE = 11;
const fld_t FLD_BYTES = 12;
const fld_t FLD_UINT32 = 13;
const fld_t FLD_ENUM = 14;
const fld_t FLD_SFIXED32 = 15;
const fld_t FLD_SFIXED64 = 16;
const fld_t FLD_SINT32 = 17;
const fld_t FLD_SINT64 = 18;

const zz_t ZZ_FLD_LOOKUP_CUTOFF = 1024;

class Fld {
 public:
  int num;
  wire_t wire_type;
  fld_t fld_type;
};

class Msg {
 public:
  std::string name;
  virtual void add_fld(Fld fld) = 0;
  virtual Fld *get_fld(int fld_num) = 0;
  
 private:
  std::vector<Fld> flds;
};

const Msg create_msg(std::string name, zz_t max_zz_fld_num);

class ScanningMsg : public Msg {
 public:
  ScanningMsg(std::string name);
  void add_fld(Fld fld);
  Fld *get_fld(int fld_num);
};

class IndexingMsg : public Msg {
 public:
  IndexingMsg(std::string name, zz_t max_zz_fld_num);
  void add_fld(Fld fld);
  Fld *get_fld(int fld_num);
};

class Model {
 public:

  std::vector<Msg> msgs;
};

#define MODEL(handle) (Model*)NUM2LONG(handle);

zz_t max_zz_field(VALUE fields);
zz_t zz_enc32(int n);

inline VALUE rb_get(VALUE receiver, const char* name) {
  return rb_funcall(receiver, rb_intern(name), 0);
}


#endif
