class Pbr

  module Enum
    def values
      self.constants.map { |sym| self.const_get(sym) }
    end
  end

  module TFieldType
    extend Enum
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

  module Label
    extend Enum
    OPTIONAL = 1
    REQUIRED = 2
    REPEATED = 3
  end

  class TField
    attr_accessor :name, :num, :type, :msg_class, :label, :packed

    def initialize(name, num, type, opts={})
      @name, @num, @type = name, num, type
      @msg_class = opts[:msg_class]
      @label = opts[:label] || Label::REQUIRED
      @packed = opts[:packed]
    end
  end

end
