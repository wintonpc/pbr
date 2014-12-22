require 'pbr'

class Everything
  include Pbr::Message

  required :f_int32, :int32, 1
  required :f_int64, :int64, 2
  required :f_uint32, :uint32, 3
  required :f_uint64, :uint64, 4
end
