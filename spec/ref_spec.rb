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
    )
    decoded = roundtrip(msg)
    expect(decoded.f_int32).to eql 2**31 - 1
    expect(decoded.f_int64).to eql 2**63 - 1
    expect(decoded.f_uint32).to eql 2**32 - 1
    expect(decoded.f_uint64).to eql 2**64 - 1
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
