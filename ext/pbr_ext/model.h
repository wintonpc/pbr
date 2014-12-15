#ifndef model_h__
#define model_h__

#include <ruby.h>
#include <vector>
#include <string>
#include "common.h"

struct Model;
struct Msg;
struct Fld;

typedef void (*add_fld_func)(Msg*, Fld);
typedef Fld* (*get_fld_func)(Msg*, int);

struct Model {
  std::vector<Msg> msgs;
};

struct Msg {
  std::string name;
  VALUE target;
  std::vector<Fld> flds;
  add_fld_func add_fld;
  get_fld_func get_fld;
};

struct Fld {
  int num;
  std::string name;
  wire_t wire_type;
  fld_t fld_type;  
};

Msg make_msg(std::string name, zz_t max_zz_fld_num);
Msg* get_msg_by_name(Model* model, std::string name);

#endif
