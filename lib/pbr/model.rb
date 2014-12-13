class Pbr

  module TFieldType
    ## 0 is reserved for errors.
    ## Order is weird for historical reasons.
    DOUBLE   = 1
    FLOAT    = 2
    INT64    = 3 ## Not ZigZag encoded.  Negative numbers
                 ## take 10 bytes.  Use TYPE_SINT64 if negative
                 ## values are likely.
    UINT64   = 4
    INT32    = 5 ## Not ZigZag encoded.  Negative numbers
                 ## take 10 bytes.  Use TYPE_SINT32 if negative
                 ## values are likely.
    FIXED64  = 6
    FIXED32  = 7
    BOOL     = 8
    STRING   = 9
    GROUP    = 10 ## Tag-delimited aggregate.
    MESSAGE  = 11 ## Length-delimited aggregate.

    ## New in version 2.
    BYTES    = 12
    UINT32   = 13
    ENUM     = 14
    SFIXED32 = 15
    SFIXED64 = 16
    SINT32   = 17 ## Uses ZigZag encoding.
    SINT64   = 18

     ## Uses ZigZag encoding.
  end

  TField = Struct.new(:name, :type)

  TMessage = Struct.new(:fields)

end