# pbr

# Overview

Class-agnostic types are generated from protobuf files via `protoc` plugin facility.
An instance of Pbr is constructed with a _rule_. A rule expresses a mapping of
PB types onto ruby classes. A mapping may target either hashes or "normal" classes
with getter/setter attributes. The rule also has a field mapping that specifies how
to inflect field names (or hash keys).

# To do

- performance testing
  - test function pointers vs big `switch`

- hash support
  - consider being more flexible with determining if a target type should be treated as a hash
    (for things that act like a hashes but are not)

- validation
  - when writing, verify value type
  - when reading, count actual required fields vs expected. only compute details if actual != expected.
    - must take into account the possibility of required/expected fields being repeated too ?
  - raise error

- support nested serialized protobuf messages (for metadata-wrapped messages)
  - additional field option on `bytes` field contains pbr_type

- optimization
  - pointers vs references
  - track average (or max) message byte-size per type.
    - use to estimate byte size of varint embedded msg length.
      if we guess correctly, we can write the embedded message in place
      and back-fill the length, instead of writing to a temp buffer,
      finding the length, then copying. Pad with one extra varint byte
      of zeros to reduce likelihood of having to memmove.
    - also use to estimate initial capacity of top level message.
    - rethought:
      remember last encoded size for each message type (on both write and read??),
      as well as the number of bytes it took to encode the size as a varint.
      (this is cheaper than averaging, and probably just as good)
      - when writing a type:
        - use the previous size as a basis for initializing the buffer capacity
        - if embedded, allocate the previously required number of bytes
          for the varint length.
          - if this proves to be more than actually needed, pad with zero bits
          - if not enough, insert the appropriate number of additional bytes.
            (std::vector.insert() is likely performant enough for this)
  - reuse the same vector for temporary embedded message writing

