class Pbr

  module TFieldType
    ## 0 is reserved for errors.
    DOUBLE   = 1
    FLOAT    = 2
    INT64    = 3
    UINT64   = 4
    INT32    = 5
    FIXED64  = 6
    FIXED32  = 7
    BOOL     = 8
    STRING   = 9
    GROUP    = 10
    MESSAGE  = 11
    BYTES    = 12
    UINT32   = 13
    ENUM     = 14
    SFIXED32 = 15
    SFIXED64 = 16
    SINT32   = 17
    SINT64   = 18
  end

  TField = Struct.new(:name, :num, :type, :msg_class)

  TMessage = Struct.new(:name, :fields)

end
