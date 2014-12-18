# pbr

# Overview

Class-agnostic types are generated from protobuf files via `protoc` plugin facility.
An instance of Pbr is constructed with a _rule_. A rule expresses a mapping of
PB types onto ruby classes. A mapping may target either hashes or "normal" classes
with getter/setter attributes. The rule also has a field mapping that specifies how
to inflect field names (or hash keys).

# To do

- embedded messages
- repeated, optional

- test against another PB implementation

- hash support
  - consider being more flexible with determining if a target type should be treated as a hash
    (for things that act like a hashes but are not)

- validation
  - count actual required fields vs expected. only compute details if actual != expected.
  - raise error

- make type declarations friendlier
  - implement `protoc` plugin

- consider ObjectId -> string problem

- support nested serialized protobuf messages (for metadata-wrapped messages)
  - additional field option on `bytes` field contains pbr_type

- optimization
  - pointers vs references


