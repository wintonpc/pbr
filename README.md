# pbr
[![Build Status](https://travis-ci.org/wintonpc/pbr.svg)](https://travis-ci.org/wintonpc/pbr)

PBR (or ProtoBuf for Ruby) is a pragmatic Ruby interface to Google's ProtocolBuffers.
The majority of the code is implemented as a C++ Ruby extension.

Goals include:

- speed, possibly at the expense of memory footprint (within reason),
- de/serialization to/from arbitrary types, including hashes (to avoid middleman inefficiencies),
- arbitrary de/inflation of field types not supported by Protobuf, e.g., `Time` and `BSON::ObjectId`, and
- hacked polymorphism support based on lazy submessage deserialization.

The [beefcake](https://rubygems.org/gems/beefcake) gem was used as a starting point for the Ruby-facing code,
so moving to Pbr should be a smooth transition if you're familiar with beefcake.

# Overview

Use protoc plugin to generate ruby type descriptors from `.proto` files

```bash
protoc --proto_path=protos --pbr_out=lib/messages protos/my_message.proto
```

resulting in

```ruby
# lib/messages/my_message.pb.rb
class MyMessage
  include Pbr::Message

  required :foo, :string, 1
  optional :bar, :int32, 2
  repeated :baz, :sint64, 3
end
```

Use it:

```ruby
pbr = Pbr.new
msg = MyMessage.new(foo: 'hello', bar: 42, baz: -1)
serialized = pbr.write(msg, MyMessage)
deserialized = pbr.read(serialized, MyMessage)
```

# Features

### Object instantiation

```ruby
SomeMessage.new(foo: 'hello', bar: {a: 1, b: 2})
```

Objects can be initialized with an attributes hash.
Embedded messages can be initialized with subhashes, as with `bar` in this example.

### Pbr instantiation

As illustrated above, Pbr can de/serialize to/from the generated classes. Pbr can also target
arbitrary types.

A pbr object is instantiated with a `PbrMapping` object. `PbrMapping` describes a mapping from
the set of generated _source_ types to a set of arbitrary _target_ types. By default,
Pbr uses mapping `PbrMapping.vanilla`, which maps each generated type to itself.
`PbrMapping.always(some_type)` maps each generated type to
`some_type`. For instance, `PbrMapping.always(OpenStruct)` realizes protobuf messages as OpenStruct
objects. `PbrMapping.hash_with_string_keys` and `PbrMapping.hash_with_symbol_keys` are self-explanatory.

You can create custom `PbrMapping` objects to express arbitrary mappings.

```ruby
# a possible implementation of PbrMapping.vanilla
PbrMapping.new do |m|
  m.get_target_type = proc {|source_type| source_type}
  m.get_target_field = proc {|field| field.name}
end

# a possible implementation of PbrMapping.hash_with_string_keys
PbrMapping.new do |m|
  m.get_target_type = proc {|source_type| Hash}
  m.get_target_key = proc {|field| field.name.to_s}
end
```

Target types are manipulated by assuming the presence of `field_name` getters and `field_name=` setters.
For `Hash` target types, the value returned by `get_target_key` is used as the key.

### Field Inflation/Deflation

Field types that are not directly supported in ProtoBuf can be deflated upon serialization and
inflated upon deserialization.

```ruby
class TestMsg
  include Pbr::Message
  required :stamp, :string, 1  # this field contains a `Time` in Ruby, but is serialized as a string

  deflate(:stamp) {|time| time.utc.iso8601(3)}
  inflate(:stamp) {|iso_str| Time.parse(iso_str).localtime}
end
```

The recommended way to add inflators/deflators is to reopen the class in a separate file, so as to
avoid modifying the generated `.pb.rb` file.

### Polymorphism

One design tradeoff in ProtoBuf is that messages are not self-describing, in the sense that you
cannot determine the type of a message by inspecting its serialized form. You can easily
work around this problem with a wrapper message.
(There are [other options](http://www.indelible.org/ink/protobuf-polymorphism/).)

```ruby
class WrappedMsg
  include Pbr::Message
  required :type, :string, 1
  required :msg, :bytes, 2   # the serialized message
end
```

To read such a message, the first step is to read the `type` field to learn the type of the wrapped message.
The second step is to deserialize `msg` as that type. Ruby's garbage collector is
[notoriously poor](https://www.google.com/search?q=ruby+string+garbage+collection)
at managing strings efficiently. Pbr supports this case in pure C++. This avoids allocating a Ruby string for the
`msg` field, only to pass it back to the C++ extension for deserialization. Performance is the same as
a normal non-polymorphic embedded message field.

```ruby
class WrappedMsg
  include Pbr::Message
  required :type, :string, 1
  required :msg, :bytes, 2   # the serialized message

  type_of(:msg) {|wrapped| wrapped.type.constantize }  # constantize from activesupport
end
```

You must preregister all possible polymorphic types ahead of time with `Pbr#register`.

### Quirks

- Unrecognized fields are discarded upon deserialization. Consequently, Pbr should not be used
  when passthrough behavior is required.
- unsupported
  - default values for optional fields
  - extensions
  - 'oneof'
- A Float in an integer-typed field is truncated when serializing, as with Float::to_i
- The validation check for missing required fields when reading a message may exhibit false negatives
  when required fields are duplicated in the protobuf. Duplicate fields are unlikely.
  Validating on read is somewhat dubious anyway; presumably the originator would have
  detected the error on write.

