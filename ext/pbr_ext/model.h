#ifndef model_h__
#define model_h__

#include <ruby.h>
#include <vector>
#include <string>
#include "common.h"
#include "types.h"
#include "ss.h"

struct Model;
struct Msg;
struct Fld;

typedef void (*add_fld_func)(Msg*, Fld);
typedef Fld* (*get_fld_func)(Msg*, fld_num_t);

typedef VALUE (*write_obj_func)(Msg* msg, buf_t& buf, VALUE obj);
typedef void (*write_fld_func)(buf_t& buf, VALUE obj, Fld* fld);

typedef VALUE (*read_obj_func)(Msg* msg, ss_t& ss);
typedef void (*read_fld_func)(ss_t& ss, VALUE obj, ID target_field_setter);
typedef void (*read_key_func)(ss_t& ss, VALUE obj, VALUE target_key);

struct Model {
  std::vector<Msg> msgs;
};

struct Msg {
  int index;
  std::string name;
  VALUE target;
  bool target_is_hash;
  std::vector<Fld> flds_to_lookup;
  std::vector<Fld> flds_to_enumerate;
  add_fld_func add_fld;
  get_fld_func get_fld;
  write_obj_func write;
  read_obj_func read;
};

struct Fld {
  fld_num_t num;
  std::string name;
  ID target_field;
  ID target_field_setter;
  VALUE target_key;
  wire_t wire_type;
  fld_t fld_type;
  Msg* embedded_msg;
  write_fld_func write_fld;
  read_fld_func read_fld;
  read_key_func read_key;
};


Msg make_msg(std::string name, fld_num_t max_fld_num);
Msg* get_msg_by_name(Model* model, std::string name);

#endif
