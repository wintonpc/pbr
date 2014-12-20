#ifndef types_h__
#define types_h__

#include <cstdint>
#include <vector>

typedef int wire_t;
typedef int fld_t;
typedef int label_t;
typedef uint32_t fld_num_t;
typedef unsigned int zz32_t;
typedef long unsigned int zz64_t;
typedef std::vector<unsigned char> buf_t;

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

const label_t LABEL_OPTIONAL = 1;
const label_t LABEL_REQUIRED = 2;
const label_t LABEL_REPEATED = 3;

#endif
