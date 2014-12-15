# pbr

# Notes

```ruby
obj = Pbr.read(buf, pbr_type)
buf = Pbr.write(obj, pbr_type?)
```

A `pbr_type` is a beefcake ruby class and another ruby class;

- leverage beefcake
- do reflection in ruby

- to initialize:
  - get a handle from C
    - for each beefcake type, generate

      - message type
        - FQ name ?
        - field types
        - max field num (zigzagged)

      - field type
        - value type (int32, int64, etc.)
        - wire type (0-5)
        - rule (required, optional, repeated, repeated_packed)
        - default?

- when encountering a new message type (on serialize or deserialize)
  - map the type to a copy of the corresponding message type, augmented with getters and setters
  - getters and setters are symbols to send to the target ruby object
  - getters and setters are in an array, indexed by the zigzagged field num

- support hashes

- support nested serialized protobuf messages (for metadata-wrapped messages)
  - additional field option on `bytes` field contains pbr_type

- to read
  - ensure handle
  - ensure pbr_type initialized

- per-pbr_type settings
  - field name inflection options
    - allow arbitrary mappings with lambdas

- optimization
  - change enc/decode-time functions to use references
