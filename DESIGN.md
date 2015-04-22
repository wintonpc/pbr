## Protobuf Specs

[Encoding](https://developers.google.com/protocol-buffers/docs/encoding)

[Protobuf compiler plugin API](https://developers.google.com/protocol-buffers/docs/reference/other).
Documentation on the meta-protos sent between the compiler and a plugin are harder to come by.
[Here](https://github.com/google/protobuf/blob/v2.6.1/src/google/protobuf/compiler/plugin.proto)
and
[here](https://github.com/google/protobuf/blob/v2.6.1/src/google/protobuf/descriptor.proto)
are good starting points.


## Optimizations

Generally, as much work as possible is done up front during type
registration, in order to decrease the amount of work required to
de/serialize. An effort is made to make validation as lightweight as
possible; however, once validation for a message starts to fail, less
emphasis is placed on optimizing the reporting of errors. In most
well-designed applications, such errors would be rare and occur mostly
during development.

### Field lookup

When reading a message, each field is preceded by a header that
contains the field number. The metafield must be looked up to know how
to deserialize the field. To make this lookup efficient, message types
with reasonably low field numbers store their metafields in a sparse
vector. This allows the metafield to be looked up by directly indexing
into the vector. For messages with large field numbers, metafields are
stored in a dense vector and lookup is done by scanning.

### Function pointers

The typical way to determine which code to execute to de/serialize a
field would probably be with a `switch` on field type. An alternative
is to have each metafield store a function pointer to the appropriate
code, as determined during type registration. The function pointer
approach has shown to be marginally faster for Pbr. (If `switch` made
the code significantly more readable, I would opt for it; but it does
not.)

### Embedded messages

An embedded message is prefixed with the varint encoded length of the
message. To know how many bytes the varint will take, you need to know
the encoded message length; to know the length, you must do the
encoding. The obvious approach is to write the embedded message to a
temporary buffer; then write the length to the primary buffer, and
copy the embedded message from the temporary buffer. This results in
costly memory allocations and copying. An embedded message may even be
copied more than once, in proportion to its depth within the message
structure.

To minimize such memory operations, each metamessage remembers the
length of its last serialized instance, specifically, the number of
bytes it took to encode the length as a varint. When writing an
embedded message, we assume the same number of bytes will be needed
for the varint length. Space in the primary buffer is preallocated for
the length, and the embedded message is written directly to the
primary buffer. Afterward, the preallocated bytes are filled in with
the actual length. If too many bytes were preallocated, the extras are
padded with zeros according to the varint algorithm. If too few were
preallocated, bytes are inserted (with `std::vector.insert`) to
accomodate the varint. With typical messages, this results in a low
number of memmov operations on average, and no temporary buffer
allocation.

### Packed repeated fields

Similarly to embedded messages, packed repeated fields prefix their
data with a varint length. Since packed fields are an optimization for
large array fields, saving one byte per element, it makes sense to
preallocate the maximum varint size (5 bytes) to hold the length. The
overhead is insignificant given the (assumed large) number of repeated
values, and we are guaranteed that no memmov will be required.

### Garbage Collection

The C++ extension holds on to ruby object references (VALUEs) -- for
instance, the 'deflate' procedures -- but ruby's garbage collector is
unaware of these references. If they get collected, all kinds of
random nastiness ensues. Consequently, all VALUEs held in C++ are also
added to a ruby array held by the Pbr instance. This ensures they are
not collected until the C++ code no longer needs them.


# Test strategy

- Verify Pbr preserves messages through read/write roundtrips
  - use an exhaustive selection of field types
  - test extreme values for each type
- Verify Pbr can talk to existing protobuf implementations
  - communicate with a Java process
  - java process verifies it sees the right data
  - ruby side verifies it sees the right data echoed from java
- Monitor performance throughout development
  - encode big messages with a large variety of field types
  - use large, real-life messages
- Manually test for memory leaks
- Test descriptor validation
- Test message validation

