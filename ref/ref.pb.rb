require 'pbr'

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
end
