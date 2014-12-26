## Generated from test.proto for com.indigobio.test
require 'pbr'

class BigHelper
  include Pbr::Message
end

class BigMsg
  include Pbr::Message

  module State
    include Pbr::Enum
    ACTIVE = 1
    INACTIVE = 2
  end
end

class BigHelper
  repeated :a, :sint32, 1
end

class BigMsg
  repeated :f_int32, :int32, 1
  repeated :f_int64, :int64, 2
  repeated :f_uint32, :uint32, 3
  repeated :f_uint64, :uint64, 4
  repeated :f_sint32, :sint32, 5
  repeated :f_sint64, :sint64, 6
  repeated :f_bool, :bool, 7
  repeated :f_enum, BigMsg::State, 8
  repeated :f_fixed64, :fixed64, 9
  repeated :f_sfixed64, :sfixed64, 10
  repeated :f_double, :double, 11
  repeated :f_string, :string, 12
  repeated :f_bytes, :bytes, 13
  repeated :f_embedded, BigHelper, 14
  repeated :f_packed, :int32, 15, packed: true
  repeated :f_fixed32, :fixed32, 16
  repeated :f_sfixed32, :sfixed32, 17
  repeated :f_float, :float, 18
end
