require 'rspec'
require 'ostruct'
require 'pbr'
require 'test_msg'
require 'active_support/inflector'

describe Pbr do

  context 'roundtrips' do
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

    it 'int32' do
      roundtrip(:int32, 0)
      roundtrip(:int32, 1)
      roundtrip(:int32, -1)
      roundtrip(:int32, 1234)
      roundtrip(:int32, 2**31 - 1)
      roundtrip(:int32, -2**31)
    end

    it 'uint32' do
      roundtrip(:uint32, 0)
      roundtrip(:uint32, 1)
      roundtrip(:uint32, 1234)
      roundtrip(:uint32, 2**32 - 1)
    end

    it 'int64' do
      roundtrip(:int64, 0)
      roundtrip(:int64, 1)
      roundtrip(:int64, -1)
      roundtrip(:int64, 1234)
      roundtrip(:int64, 2**63 - 1)
      roundtrip(:int64, -2**63)
    end

    it 'uint64' do
      roundtrip(:uint64, 0)
      roundtrip(:uint64, 1)
      roundtrip(:uint64, 1234)
      roundtrip(:uint64, 2**64 - 1)
    end

    it 'sint32' do
      roundtrip(:sint32, 0)
      roundtrip(:sint32, 1)
      roundtrip(:sint32, -1)
      roundtrip(:sint32, 1234)
      roundtrip(:sint32, 2**31 - 1)
      roundtrip(:sint32, -2**31)
    end

    it 'sint64' do
      roundtrip(:sint64, 0)
      roundtrip(:sint64, 1)
      roundtrip(:sint64, -1)
      roundtrip(:sint64, 1234)
      roundtrip(:sint64, 2**63 - 1)
      roundtrip(:sint64, -2**63)
    end

    it 'sfixed32' do
      roundtrip(:sfixed32, 0)
      roundtrip(:sfixed32, 1)
      roundtrip(:sfixed32, -1)
      roundtrip(:sfixed32, 1234)
      roundtrip(:sfixed32, 2**31 - 1)
      roundtrip(:sfixed32, -2**31)
    end

    def roundtrip(short_message_type, str, field_name=:foo, field_num=1)
      message_type = msg_type(short_message_type, field_name, field_num)
      obj = OpenStruct.new
      obj.send("#{field_name}=", str)
      bytes = Pbr.new.write(obj, message_type)
      obj2  = Pbr.new.read(bytes, message_type)
      v2    = obj2.send(field_name)
      v1    = obj.send(field_name)
      shown = "#{v1.inspect} -> #{v2.inspect}"
      puts shown.size > 120 ? shown[0..100] + '...' : shown
      expect(v2).to eql v1
    end
    def msg_type(field_type, field_name=:foo, field_num=1)
      field_type_class = "Pbr::TFieldType::#{field_type.to_s.upcase}".constantize
      Pbr::TMessage.new('TestMsg', [Pbr::TField.new(field_name, field_num, field_type_class)])
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