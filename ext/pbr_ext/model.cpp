#include "model.h"
#include "encode.h"


void add_indexing_fld(Msg& msg, Fld fld) {
  msg.flds[zz_enc32(fld.num)] = fld;
}

const Fld* get_indexing_fld(const Msg& msg, int fld_num) {
  zz_t zz_fld_num = zz_enc32(fld_num);
  if (zz_fld_num >= msg.flds.size())
    return NULL;
  else
    return &msg.flds[zz_fld_num];
}

void add_scanning_fld(Msg& msg, Fld fld) {
  msg.flds.push_back(fld);
}

const Fld* get_scanning_fld(const Msg& msg, int fld_num) {
  int len = msg.flds.size();
  for (int i=0; i<len; i++)
    if (msg.flds[i].num == fld_num)
      return &msg.flds[i];
  return NULL;
}

Msg make_msg(std::string name, zz_t max_zz_fld_num) {
  Msg msg;
  msg.name = name;
  
  if (max_zz_fld_num <= ZZ_FLD_LOOKUP_CUTOFF) {
    msg.flds.reserve(max_zz_fld_num);
    msg.add_fld = add_scanning_fld;
    msg.get_fld = get_scanning_fld;
  }
  else {
    msg.add_fld = add_indexing_fld;
    msg.get_fld = get_indexing_fld;
  }

  return msg;
}
