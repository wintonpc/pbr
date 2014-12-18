require 'rspec'
require 'ostruct'
require 'pbr'
require 'test_msg'
require 'active_support/inflector'

describe Pbr do

  context 'roundtrips' do

    def roundtrip(tripper, short_message_type, str, field_name=:foo, field_num=1)
      v1, v2 = do_roundtrip(tripper, short_message_type, str, field_name=:foo, field_num=1)
      expect(v2).to eql v1
    end

    def roundtrip_float(tripper, precision, short_message_type, str, field_name=:foo, field_num=1)
      v1, v2 = do_roundtrip(tripper, short_message_type, str, field_name=:foo, field_num=1)
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

    def do_roundtrip(tripper, short_message_type, str, field_name=:foo, field_num=1)
      message_type = msg_type(short_message_type, field_name, field_num)
      v1, v2 = tripper.call(message_type, str, field_name)
      shown = "#{v1.inspect}#{v1.is_a?(String) ? " (#{v1.encoding})" : ''} -> #{v2.inspect}#{v2.is_a?(String) ? " (#{v2.encoding})" : ''}"
      puts shown.size > 120 ? shown[0..100] + '...' : shown
      [v1, v2]
    end

    def self.do_obj_roundtrip(message_type, field_val, field_name=:foo)
      obj = OpenStruct.new
      obj.send("#{field_name}=", field_val)
      bytes = Pbr.new.write(obj, message_type)
      obj2  = Pbr.new.read(bytes, message_type)
      # expect(obj2).to be_a OpenStruct
      v2    = obj2.send(field_name)
      v1    = obj.send(field_name)
      [v1, v2]
    end

    def self.do_hash_roundtrip(message_type, field_val, key_name=:foo)
      obj = {}
      obj[key_name] = field_val
      bytes = Pbr.new.write(obj, message_type)
      obj2  = Pbr.new.read(bytes, message_type)
      # expect(obj2).to be_a Hash
      v2    = obj2[key_name]
      v1    = obj2[key_name]
      [v1, v2]
    end

    def msg_type(field_type, field_name=:foo, field_num=1)
      field_type_class = "Pbr::TFieldType::#{field_type.to_s.upcase}".constantize
      sub_msg = Pbr::TMessage.new('SubMsg', [Pbr::TField.new('bar', field_num, field_type_class)])
      test_msg = Pbr::TMessage.new('TestMsg', [
                                      Pbr::TField.new(field_name, field_num, field_type_class),
                                      Pbr::TField.new('sub', field_num + 1, sub_msg)
                                 ])
      test_msg
    end

    def self.it_roundtrips_int(tripper, bits, type)
      it "#{type} with #{tripper.name}" do
        roundtrip(tripper, type, 0)
        roundtrip(tripper, type, 1)
        roundtrip(tripper, type, -1)
        roundtrip(tripper, type, 1234)
        roundtrip(tripper, type, 2**(bits-1) - 1)
        roundtrip(tripper, type, -2**(bits-1))
      end
    end

    def self.it_roundtrips_uint(tripper, bits, type)
      it "#{type} with #{tripper.name}" do
        roundtrip(tripper, type, 0)
        roundtrip(tripper, type, 1)
        roundtrip(tripper, type, 1234)
        roundtrip(tripper, type, 2**bits - 1)
      end
    end

    [method(:do_obj_roundtrip)].each do |t|

      it "field names with #{t.name}" do
        roundtrip(t, :string, 'sss', :foo)
        roundtrip(t, :string, 'sss', 'foo')
      end

      it "field numbers with #{t.name}" do
        min_field_num = 1
        max_field_num = 2 ** 29 - 1
        roundtrip(t, :string, 'sss', :foo, min_field_num)
        roundtrip(t, :string, 'sss', :foo, 1234)
        roundtrip(t, :string, 'sss', :foo, max_field_num)
      end

      it "string with #{t.name}" do
        roundtrip(t, :string, '')
        roundtrip(t, :string, 'hello, world!')
        roundtrip(t, :string, "hello\0world")
        roundtrip(t, :string, 'z' * 1024 * 1024)

        v1, v2 = do_roundtrip(t, :string, 'hello, world!'.encode('utf-16'))
        expect(v1.encoding).to eql Encoding::UTF_16
        expect(v2.encoding).to eql Encoding::UTF_8
        expect(v2).to eql v1.encode('utf-8')
      end

      it "bytes with #{t.name}" do
        roundtrip(t, :bytes, 'hello')
        roundtrip(t, :bytes, [0, 1, 2, 3, 255].pack('c*'))
      end

      it "float with #{t.name}" do
        roundtrip_float(t, 7, :float, 0.0)
        roundtrip_float(t, 7, :float, 3.14)
        roundtrip_float(t, 7, :float, -3.14)
        roundtrip_float(t, 7, :float, 3.402823e38)
        roundtrip_float(t, 7, :float, -3.402823e38)
      end

      it "double with #{t.name}" do
        roundtrip(t, :double, 0.0)
        roundtrip(t, :double, 3.14)
        roundtrip(t, :double, -3.14)
        roundtrip(t, :double, 3.402823e38)
        roundtrip(t, :double, -3.402823e38)
        roundtrip(t, :double, 0.030000000000000006)
      end

      it "bool with #{t.name}" do
        roundtrip(t, :bool, true)
        roundtrip(t, :bool, false)
      end

      it_roundtrips_int(t, 32, :int32)
      it_roundtrips_uint(t, 32, :uint32)
      it_roundtrips_int(t, 64, :int64)
      it_roundtrips_uint(t, 64, :uint64)
      it_roundtrips_int(t, 32, :sint32)
      it_roundtrips_int(t, 64, :sint64)
      it_roundtrips_int(t, 32, :sfixed32)
      it_roundtrips_uint(t, 32, :fixed32)
      it_roundtrips_int(t, 64, :sfixed64)
      it_roundtrips_uint(t, 64, :fixed64)

      it "embedded messages with #{t.name}" do

      end
    end
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