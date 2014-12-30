require 'rspec'
require 'pbr'
require 'oj'
require 'securerandom'
require_relative 'bigmsg.pb'

describe 'Pbr' do

  it 'should be performant' do
    pbr = Pbr.new
    n = 10000
    pb_msg = BigMsg.new(
        f_int32: repeat(n){rand_int(32)},
        f_int64: repeat(n){rand_int(64)},
        f_uint32: repeat(n){rand_uint(32)},
        f_uint64: repeat(n){rand_uint(64)},
        f_sint32: repeat(n){rand_int(32)},
        f_sint64: repeat(n){rand_int(64)},
        f_bool: repeat(n){Random.rand(0..1) == 0},
        f_enum: repeat(n){Random.rand(1..2)},
        f_fixed64: repeat(n){rand_uint(64)},
        f_sfixed64: repeat(n){rand_int(64)},
        f_double: repeat(n){Random.rand},
        f_string: repeat(n){SecureRandom.uuid},
        f_bytes: repeat(n){20.times.map{Random.rand(0..255)}},
        f_embedded: repeat(n){BigHelper.new(a: 10.times.map{rand_int(32)})},
        f_packed: repeat(n){rand_int(32)},
        f_fixed32: repeat(n){rand_uint(32)},
        f_sfixed32: repeat(n){rand_int(32)},
        f_float: repeat(n){Random.rand}
    )

    pb_encoded = nil
    pbr.register(BigMsg)
    pb_time = time_it(:pb_roundtrip) do
      pb_encoded = pbr.write(pb_msg, BigMsg)
      pbr.read(pb_encoded, BigMsg)
    end

    oj_msg = Pbr.new(PbrMapping.always(OpenStruct)).read(pb_encoded, BigMsg).to_h

    oj_encoded = nil
    oj_time = time_it(:oj_roundtrip) do
      oj_encoded = Oj.dump(oj_msg)
      Oj.load(oj_encoded)
    end

    report('protobuf', pb_encoded.size, pb_time)
    report('oj', oj_encoded.size, oj_time)
  end

  def report(name, byte_size, elapsed_seconds)
    puts "#{name.to_s.ljust(10)} #{(byte_size / 1024).to_s.rjust(4)} KB  " +
             "#{(elapsed_seconds * 1000).round.to_s.rjust(4)} ms  read+write"
  end

  def rand_int(bits)
    mag = 2**(bits-1)
    Random.rand(-mag...mag)
  end

  def rand_uint(bits)
    mag = 2**bits
    Random.rand(0...mag)
  end

  def repeat(n, &make_val)
    n.times.map{yield}
  end

  def time_it(name)
    GC.disable
    start = Time.now
    yield
    stop = Time.now
    GC.enable
    elapsed_seconds = stop - start
    puts "==> #{name} took #{elapsed_seconds} seconds"
    elapsed_seconds
  end
end
