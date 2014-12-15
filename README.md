# pbr

# Notes

Class-agnostic types are generated from protobuf files via `protoc` plugin facility.
An instance of Pbr is constructed with a _rule_. A rule expresses a mapping of
PB types onto ruby classes. A mapping may target either hashes or "normal" classes
with getter/setter attributes. The rule also has a field mapping that specifies how
to inflect field names (or hash keys).

```ruby
pbr = Pbr.new(PbrRule.default)
obj = Pbr.read(buf, pbr_type)
buf = Pbr.write(obj, pbr_type)
```

A `pbr_type` is a beefcake ruby class and another ruby class:

- hash support
  - consider being more flexible with determining if a target type should be treated as a hash

- support nested serialized protobuf messages (for metadata-wrapped messages)
  - additional field option on `bytes` field contains pbr_type

- optimization
  - change enc/decode-time functions to use references
