require 'active_support/inflector'

# borrowed from beefcake

class FieldOptions
  include Pbr::Message
end

class FieldDescriptorProto
  include Pbr::Message

  module Type
    include Pbr::Enum
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

  module Label
    include Pbr::Enum
    OPTIONAL = 1
    REQUIRED = 2
    REPEATED = 3
  end
end

class FieldOptions
  optional :packed, :bool, 2
end

class FieldDescriptorProto

  optional :name,   :string, 1
  optional :number, :int32,  3
  optional :label,  Label,  4

  ## If type_name is set, this need not be set.  If both this and type_name
  ## are set, this must be either TYPE_ENUM or TYPE_MESSAGE.
  optional :type, Type, 5

  ## For message and enum types, this is the name of the type.  If the name
  ## starts with a '.', it is fully-qualified.  Otherwise, C++-like scoping
  ## rules are used to find the type (i.e. first the nested types within this
  ## message are searched, then within the parent, on up to the root
  ## namespace).
  optional :type_name, :string, 6

  ## For extensions, this is the name of the type being extended.  It is
  ## resolved in the same manner as type_name.
  optional :extended, :string, 2

  ## For numeric types, contains the original text representation of the value.
  ## For booleans, "true" or "false".
  ## For strings, contains the default text contents (not escaped in any way).
  ## For bytes, contains the C escaped value.  All bytes >= 128 are escaped.
  optional :default_value, :string, 7

  optional :options, FieldOptions, 8
end

class EnumValueDescriptorProto
  include Pbr::Message

  optional :name,   :string, 1
  optional :number, :int32,  2
  # optional EnumValueOptions options = 3;
end

class EnumDescriptorProto
  include Pbr::Message

  optional :name, :string, 1
  repeated :value, EnumValueDescriptorProto, 2
  # optional :options, EnumOptions, 3
end

class DescriptorProto
  include Pbr::Message

  optional :name, :string, 1

  repeated :field,       FieldDescriptorProto, 2
  repeated :extended,    FieldDescriptorProto, 6
  repeated :nested_type, DescriptorProto,      3
  repeated :enum_type,   EnumDescriptorProto,  4
end


class FileDescriptorProto
  include Pbr::Message

  optional :name, :string, 1       # file name, relative to root of source tree
  optional :package, :string, 2    # e.g. "foo", "foo.bar", etc.
  repeated :dependencies, :string, 3

  repeated :message_type, DescriptorProto,     4
  repeated :enum_type,    EnumDescriptorProto, 5
end
