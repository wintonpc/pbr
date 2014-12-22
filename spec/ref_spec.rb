require 'rspec'
require 'pbr'
require_relative '../ref/ref.pb'

describe Pbr do
  it 'should talk to a reference implementation' do
    msg = Everything.new(
        f_int32: 2**31 - 1,
        f_int64: 2**63 - 1,
        f_uint32: 2**32 - 1,
        f_uint64: 2**64 - 1,
        f_sint32: 2**31 - 1,
        f_sint64: 2**63 - 1,
        f_bool: true,
        f_enum: Everything::State::ACTIVE,
        f_fixed64: 2**64 - 1,
        f_sfixed64: 2**63 - 1,
        f_double: 3.141592,
        f_string: 'hello',
        f_bytes: [0, 1, 2, 3, 255].pack('c*'),
        f_embedded: Something.new(a: 'goodbye'),
        f_packed: [1, 1000, 1000000],
    )
    decoded = roundtrip(msg)
    expect(decoded.f_int32).to eql 2**31 - 1
    expect(decoded.f_int64).to eql 2**63 - 1
    expect(decoded.f_uint32).to eql 2**32 - 1
    expect(decoded.f_uint64).to eql 2**64 - 1
    expect(decoded.f_sint32).to eql 2**31 - 1
    expect(decoded.f_sint64).to eql 2**63 - 1
    expect(decoded.f_bool).to eql true
    expect(decoded.f_enum).to eql Everything::State::ACTIVE
    expect(decoded.f_fixed64).to eql 2**64 - 1
    expect(decoded.f_sfixed64).to eql 2**63 - 1
    expect(decoded.f_double).to eql 3.141592
    expect(decoded.f_string).to eql 'hello'
    expect(decoded.f_bytes).to eql [0, 1, 2, 3, 255].pack('c*')
    expect(decoded.f_embedded).to be_a Something
    expect(decoded.f_embedded.a).to eql 'goodbye'
    expect(decoded.f_packed).to eql [1, 1000, 1000000]
  end

  def roundtrip(msg)
    pbr     = Pbr.new
    encoded = pbr.write(msg, Everything)
    io      = IO.popen('java -jar ref/pbtest/target/pbtest-1.0-SNAPSHOT-jar-with-dependencies.jar', 'r+')
    io.write(encoded)
    io.close_write
    returned = io.read
    pbr.read(returned, Everything)
  end
end
