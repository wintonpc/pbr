# pbr

# Overview

Class-agnostic types are generated from protobuf files via `protoc` plugin facility.
An instance of Pbr is constructed with a _mapping_. A rule expresses a mapping of
PB types onto ruby classes. A mapping may target either hashes or "normal" classes
with getter/setter attributes. The mapping also has a field mapping that specifies how
to inflect field names (or hash keys).

# Notes

- TODO
  - test descriptors with same name but in different namespaces

  - more realistic performance tests with wrapped, real messages

- documentation
  - add custom `PbrMapping` example to readme
  - add doc-comments
    - fields passed to inflectors

- parking lot
  - default values for optional fields??
    - does this make sense when mapping to arbitrary types?
    - ideally, cost would be paid at field read time, not deserialize time
  - writing to/reading from a stream
  - eventmachine implications, if any

- performance findings
  - function pointers are slightly faster than big switch. changing to big switch for writes
    was 2% slower on average. expect it would be 4% slower with reads changed over too.
  - reusing the same vector for writing embedded messages added ~5% speed improvement.
    - unfortunately, this break message embedding more than 1 level deep. could fix with
      multiple temp buffers, but might not pull its weight.
  - the ability to deflate/inflate incurs negligible overhead, about 1-2%

# Design

PBR (or ProtoBuf for Ruby) is a pragmatic Ruby interface to Google's ProtocolBuffers.
The majority of the code is implemented as a C++ Ruby extension.

Goals include:

- speed, possibly at the expense of memory footprint (within reason),
- de/serialization to/from arbitrary types, including hashes (to avoid middleman inefficiencies),
- arbitrary de/inflation of field types not supported by Protobuf, e.g., `Time` and `BSON::ObjectId`, and
- hacked polymorphism support based on lazy submessage deserialization.

## Usage

Use protoc plugin to generate ruby type descriptors from `.proto` files

```bash
protoc -I=protos --pbr_out=lib/messages protos/my_message.proto
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

use

```ruby
pbr = Pbr.new
msg = MyMessage.new(foo: 'hello', bar: 42, baz: -1)
serialized = pbr.write(msg, MyMessage)
deserialized = pbr.read(serialized, MyMessage)
```

## Features

### Object instantiation

```ruby
SomeMessage.new(foo: 'hello', bar: {a: 1, b: 2})
```

Objects may be initialized with an attributes hash.
Embedded messages can be initialized with subhashes, as with `bar` in this example.

### Pbr instantiation

As seen above, Pbr can de/serialize to/from the generated types. Pbr can also target
arbitrary types.

A pbr object is instantiated with a `PbrMapping` object. `PbrMapping` describes a mapping from
the set of generated ("_source_") types to a set of arbitrary ("_target_") types. By default,
Pbr uses mapping `PbrMapping.vanilla`,
which maps each generated type to itself. `PbrMapping.always(some_type)` maps each generated type to
`some_type`. For instance, `PbrMapping.always(OpenStruct)` realizes protobuf messages as OpenStruct
objects. You may create custom `PbrMapping` objects to express arbitrary mappings.

Target types are manipulated by assuming the presence of `field_name` getters and `field_name=` setters.
For hash target types, the mapping expresses a mapping to arbitrary Ruby key values.

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

```ruby
class WrappedMsg
  required :type, :string, 1
  required :msg, :bytes, 2   # the serialized message
end
```

To read such a message, the first step is to read the `type` field to learn the type of the wrapped message.
The second step is to deserialize `msg` as that type. Ruby's garbage collector is notoriously poor at
managing strings efficiently. Pbr supports this case in pure C++, to avoid allocating a Ruby string for the
`msg` field, only to pass it back to the C++ extension for deserialization. Details TBD.

### Quirks

- Unknown fields are discarded upon deserialization. Consequently, Pbr should not be used
  when passthrough behavior is required.
- default values for optional fields are not currently supported
- A Float in an integer-typed field is truncated when serializing, as with Float::to_i
- The test for missing required fields when reading a message may exhibit false negatives
  when required fields are duplicated in the protobuf. Duplicate fields are unlikely.
  Validating on read is somewhat dubious anyway; presumably the originator would have
  detected the error on write.

## Optimizations

Generally, as much work is done up front during type registration, in order to decrease the
amount of work required to de/serialize. An effort is made to make validation as lightweight as possible;
however, once validation for a message starts to fail, less emphasis is placed on optimizing the reporting
of errors. In most well-designed applications, such errors would be rare and occur mostly during development.

### Field lookup

When reading a message, each field is preceded by a header that contains the field number.
The metafield must be looked up to know how to deserialize the field. To make this lookup
efficient, message types with reasonably low field numbers store their metafields in a sparse
vector. This allows the metafield to be looked up by directly indexing into the vector. For
other message types, metafields are stored in a dense vector and lookup is done by scanning.

### Function pointers

The typical way to determine which code to execute to de/serialize a field would probably
be with a `switch` on field type. An alternative is to have each metafield store a function
pointer to the appropriate code, as determined during type registration. The function pointer
approach has shown to be marginally faster for Pbr. (If `switch` made the code significantly
more readable, I would opt for it; but it does not.)

### Embedded messages

An embedded message is prefixed with a varint encoded length of the message. To know how many
bytes the varint will take, you need to know the encoded message length; to know the length,
you must do the encoding. The obvious approach is to write the embedded message to a temporary
buffer; then write the length to the primary buffer, and copy the embedded message from the
temporary buffer. This results in costly memory allocations and copying. An embedded message
may even be copied more than once, in proportion to its depth within the message structure.

To minimize such memory operations, each metamessage remembers the length of its last serialized
instance, specifically, the number of bytes it took to encode the length as a varint. When writing
an embedded message, we assume the same number of bytes will be needed for the varint length. Space
in the primary buffer is preallocated for the length, and the embedded message is written directly
to the primary buffer. Afterward, the preallocated bytes are filled in with the actual length.
If too many bytes were preallocated, the extras are padded with zeros according to the varint
algorithm. If too few were preallocated, bytes are inserted (with `std::vector.insert`) to
accomodate the varint. With typical messages, this results in a low number of memmov operations
on average, and no temporary buffer allocation.

### Packed repeated fields

Similarly to embedded messages, packed repeated fields prefix their data with a varint length.
Since packed fields are an optimization for large array fields, saving one byte per element,
it makes sense to preallocate the maximum varint size (5 bytes) to hold the length. The overhead
is insignificant given the (assumed large) number of repeated values, and we are guaranteed that
no memmov will be required.


# Test strategy
