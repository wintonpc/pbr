require 'pbr'

class Something
  include Pbr::Message

  required :a, :string, 1
end

class Everything
  include Pbr::Message

  module State
    include Pbr::Enum
    ACTIVE = 1
    INACTIVE = 2
  end

  required :f_int32, :int32, 1
  required :f_int64, :int64, 2
  required :f_uint32, :uint32, 3
  required :f_uint64, :uint64, 4
  required :f_sint32, :sint32, 5
  required :f_sint64, :sint64, 6
  required :f_bool, :bool, 7
  required :f_enum, ::Everything::State, 8
  required :f_fixed64, :fixed64, 9
  required :f_sfixed64, :sfixed64, 10
  required :f_double, :double, 11
  required :f_string, :string, 12
  required :f_bytes, :bytes, 13
  required :f_embedded, Something, 14
  repeated :f_packed, :int32, 15, packed: true
  required :f_fixed32, :fixed32, 16
  required :f_sfixed32, :sfixed32, 17
  required :f_float, :float, 18
  repeated :f_things, Something, 19
end
