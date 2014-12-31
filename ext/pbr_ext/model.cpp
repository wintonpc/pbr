#include <iostream>

#include "model.h"
#include "encode.h"

using namespace std;

void add_indexing_fld(Msg& msg, Fld fld) {
  msg.flds_to_enumerate.push_back(fld);
  msg.flds_to_lookup[fld.num] = fld;
}

Fld* find_indexing_fld(Msg& msg, fld_num_t fld_num) {
  if (fld_num >= msg.flds_to_lookup.size())
    return NULL;
  else {
    Fld& fld = msg.flds_to_lookup[fld_num];
    if (fld.fld_type == 0) // uninitialized field slot
      return NULL;
    return &fld;
  }
}

void add_scanning_fld(Msg& msg, Fld fld) {
  msg.flds_to_enumerate.push_back(fld);
  msg.flds_to_lookup.push_back(fld);
}

Fld* find_scanning_fld(Msg& msg, fld_num_t fld_num) {
  for (Fld& fld : msg.flds_to_lookup)
    if (fld.num == fld_num)
      return &fld;
  return NULL;
}

Msg* find_msg_by_name(Model& model, std::string name) {
  int len = model.msgs.size();
  for (int i=0; i<len; i++) {
    Msg& msg = model.msgs[i];
    if (msg.name == name)
      return &msg;
  }
  return NULL;
}

Msg* find_msg_for_type(Model& model, VALUE type) {
  return find_msg_by_name(model, type_name(type));
}

Msg make_msg(std::string name, fld_num_t max_fld_num) {
  Msg msg;
  msg.name = name;
  
  if (max_fld_num <= FLD_LOOKUP_CUTOFF) {
    msg.flds_to_lookup.resize(max_fld_num + 1);
    msg.add_fld = add_indexing_fld;
    msg.find_fld = find_indexing_fld;
  }
  else {
    msg.add_fld = add_scanning_fld;
    msg.find_fld = find_scanning_fld;
  }
  
  return msg;
}
