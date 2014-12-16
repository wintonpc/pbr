require 'rspec'
require 'ostruct'
require 'pbr'
require 'test_msg'

describe Pbr do

  describe 'roundtrips' do
    let!(:pbr) { Pbr.new }
    after(:each) do

    end
    it 'roundtrips' do
      obj = OpenStruct.new
      obj.greeting = 'hello, world!'
      bytes = Pbr.new.write(obj, TestMsg)
      puts "bytes = #{bytes.unpack('C*').inspect}"
      puts "string = #{bytes}"
      obj2 = Pbr.new.read(bytes, TestMsg)
      expect(obj2.greeting).to eql obj.greeting
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