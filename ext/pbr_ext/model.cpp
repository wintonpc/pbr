#include <iostream>

#include "model.h"
#include "encode.h"

using namespace std;

void add_indexing_fld(Msg* msg, Fld fld) {
  msg->flds[zz_enc32(fld.num)] = fld;
}

Fld* get_indexing_fld(Msg* msg, int fld_num) {
  zz_t zz_fld_num = zz_enc32(fld_num);
  if (zz_fld_num >= msg->flds.size())
    return NULL;
  else
    return &msg->flds[zz_fld_num];
}

void add_scanning_fld(Msg* msg, Fld fld) {
  msg->flds.push_back(fld);
}

Fld* get_scanning_fld(Msg* msg, int fld_num) {
  for (Fld& fld : msg->flds)
    if (fld.num == fld_num)
      return &fld;
  return NULL;
}

Msg* get_msg_by_name(Model* model, std::string name) {
  int len = model->msgs.size();
  for (int i=0; i<len; i++) {
    Msg* msg = &model->msgs[i];
    if (msg->name == name)
      return msg;
  }
  return NULL;
}

Msg make_msg(std::string name, zz_t max_zz_fld_num) {
  Msg msg;
  msg.name = name;
  
  if (max_zz_fld_num <= ZZ_FLD_LOOKUP_CUTOFF) {
    msg.flds.resize(max_zz_fld_num + 1);
    msg.add_fld = add_indexing_fld;
    msg.get_fld = get_indexing_fld;
  }
  else {
    msg.add_fld = add_scanning_fld;
    msg.get_fld = get_scanning_fld;
  }
  
  return msg;
}
