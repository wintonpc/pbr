require 'rspec'
require 'ostruct'
require 'pbr'
require 'test_msg'
require 'active_support/inflector'

describe Pbr do

  context 'roundtrips' do

    def self.it_roundtrips_int(bits, type)
      it "#{type}" do
        roundtrip(type, 0)
        roundtrip(type, 1)
        roundtrip(type, -1)
        roundtrip(type, 1234)
        roundtrip(type, 2**(bits-1) - 1)
        roundtrip(type, -2**(bits-1))
      end
    end

    def self.it_roundtrips_uint(bits, type)
      it "#{type}" do
        roundtrip(type, 0)
        roundtrip(type, 1)
        roundtrip(type, 1234)
        roundtrip(type, 2**bits - 1)
      end
    end

    def roundtrip(short_message_type, str, field_name=:foo, field_num=1)
      v1, v2 = do_roundtrip(short_message_type, str, field_name=:foo, field_num=1)
      expect(v2).to eql v1
    end

    def roundtrip_float(precision, short_message_type, str, field_name=:foo, field_num=1)
      v1, v2 = do_roundtrip(short_message_type, str, field_name=:foo, field_num=1)
      if v1 == 0
        expect(v2).to eql v1
      else
        mag1 = [Math.log10(v1.abs).floor, 1].max
        mag2 = [Math.log10(v2.abs).floor, 1].max
        puts "   (mag: #{mag1} -> #{mag2})"
        expect(mag2).to eql mag1
        sig1 = v1 / 10**mag1
        sig2 = v2 / 10**mag2
        puts "   (sig: #{sig1} -> #{sig2})"
        expect(sig2).to be_within(10 ** (-precision)).of(sig1)
      end
    end

    def do_roundtrip(short_message_type, str, field_name=:foo, field_num=1)
      message_type = msg_type(short_message_type, field_name, field_num)
      obj = OpenStruct.new
      obj.send("#{field_name}=", str)
      bytes = Pbr.new.write(obj, message_type)
      obj2  = Pbr.new.read(bytes, message_type)
      v2    = obj2.send(field_name)
      v1    = obj.send(field_name)
      shown = "#{v1.inspect} -> #{v2.inspect}"
      puts shown.size > 120 ? shown[0..100] + '...' : shown
      [v1, v2]
    end

    def msg_type(field_type, field_name=:foo, field_num=1)
      field_type_class = "Pbr::TFieldType::#{field_type.to_s.upcase}".constantize
      Pbr::TMessage.new('TestMsg', [Pbr::TField.new(field_name, field_num, field_type_class)])
    end

    it 'field names' do
      roundtrip(:string, 'sss', :foo)
      roundtrip(:string, 'sss', 'foo')
    end

    it 'field numbers' do
      min_field_num = 1
      max_field_num = 2 ** 29 - 1
      roundtrip(:string, 'sss', :foo, min_field_num)
      roundtrip(:string, 'sss', :foo, 1234)
      roundtrip(:string, 'sss', :foo, max_field_num)
    end

    it 'string' do
      roundtrip(:string, '')
      roundtrip(:string, 'hello, world!')
      roundtrip(:string, "hello\0world")
      roundtrip(:string, 'z' * 1024 * 1024)
    end

    it 'float' do
      roundtrip_float(7, :float, 0.0)
      roundtrip_float(7, :float, 3.14)
      roundtrip_float(7, :float, -3.14)
      roundtrip_float(7, :float, 3.402823e38)
      roundtrip_float(7, :float, -3.402823e38)
    end

    it 'double' do
      roundtrip(:double, 0.0)
      roundtrip(:double, 3.14)
      roundtrip(:double, -3.14)
      roundtrip(:double, 3.402823e38)
      roundtrip(:double, -3.402823e38)
      roundtrip(:double, 0.030000000000000006)
    end

    it_roundtrips_int( 32, :int32)
    it_roundtrips_uint(32, :uint32)
    it_roundtrips_int( 64, :int64)
    it_roundtrips_uint(64, :uint64)
    it_roundtrips_int( 32, :sint32)
    it_roundtrips_int( 64, :sint64)
    it_roundtrips_int( 32, :sfixed32)
    it_roundtrips_uint(32, :fixed32)
    it_roundtrips_int( 64, :sfixed64)
    it_roundtrips_uint(64, :fixed64)
  end

  C = Struct.new(:foo)

  it 'performance tests' do
    GC.disable
    obj = C.new(nil)
    hash = {foo: nil, 'foo' => nil}

    trial(:direct) do
      obj.foo = 5
    end

    obj_set_foo = ->(obj, v){obj.foo = v}
    trial(:direct_lambda) do
      obj_set_foo.call(obj, 5)
    end

    get_obj_set_foo = ->{obj_set_foo}
    trial(:direct_lambda_indirected) do
      get_obj_set_foo.call.call(obj, 5)
    end

    trial(:send) do
      obj.send(:foo=, 5)
    end

    setter = :foo=
    trial(:captured_send) do
      obj.send(setter, 5)
    end

    trial(:hash_symbol) do
      hash[:foo] = 5
    end

    hash_set_foo = ->(h, v){h[:foo] = v}
    trial(:hash_symbol_lambda) do
      hash_set_foo.call(hash, 5)
    end

    trial(:hash_string) do
      hash['foo'] = 5
    end
  end

  def trial(name, &block)
    time_it(name) do
      10000000.times(&block)
    end
  end

  def time_it(name)
    start = Time.now
    begin
      yield
    ensure
      stop = Time.now
      puts "#{name}: #{stop - start} seconds"
    end
  end
end