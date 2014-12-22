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
