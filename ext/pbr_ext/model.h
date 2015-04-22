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
struct LazyField;

typedef void (*add_fld_func)(Msg&, Fld);
typedef Fld *(*get_fld_func)(Msg&, fld_num_t);

typedef void (*write_obj_func)(buf_t& buf, Msg& msg, VALUE obj);
typedef void (*write_val_func)(buf_t& buf, Msg& msg, Fld& fld, VALUE obj, VALUE val);

typedef VALUE (*read_obj_func)(ss_t& ss, Msg& msg);
typedef VALUE (*read_val_func)(ss_t& ss, Msg& msg, Fld& fld, std::vector<LazyField> *lazy_fields);

struct Model {
  bool validate_on_write;
  bool validate_on_read;
  std::vector<Msg> msgs;
  VALUE keep_alive_array;
};

struct Msg {
  Model *model;
  int index;
  std::string name;
  VALUE target;
  bool target_is_hash;
  std::vector<Fld> flds_to_lookup;
  std::vector<Fld> flds_to_enumerate;
  add_fld_func add_fld;
  get_fld_func find_fld;
  write_obj_func write;
  read_obj_func read;
  int32_t num_required_fields;
  int32_t last_varint_size;
};

struct Fld {
  fld_num_t num;
  std::string name;
  ID target_field_getter;
  ID target_field_setter;
  VALUE target_key;
  wire_t wire_type;
  fld_t fld_type;
  label_t label;
  bool is_packed;
  Msg *embedded_msg;
  write_val_func write;
  read_val_func read;
  VALUE deflate;
  VALUE inflate;
  std::vector<VALUE> enum_values;
  VALUE enum_module;
  VALUE get_lazy_type;
};

struct LazyField {
  ss_t ss;
  Fld& fld;

  LazyField(ss_t ss, Fld& fld);
};

Msg make_msg(const std::string& name, fld_num_t max_fld_num);
Msg *find_msg_by_name(Model& model, const std::string& name);
Msg *find_msg_for_type(Model& model, VALUE type);
void keep_alive(Model& model, VALUE obj);


#endif
