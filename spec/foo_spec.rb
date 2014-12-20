require 'rspec'
require 'ostruct'
require 'pbr'
require 'test_msg'
require 'active_support/inflector'

describe Pbr do

  context 'roundtrips' do

    def self.it_roundtrips_sint(bits, type)
      it type do
        roundtrip(type, 0)
        roundtrip(type, 1)
        roundtrip(type, -1)
        roundtrip(type, 1234)
        roundtrip(type, 2**(bits-1) - 1)
        roundtrip(type, -2**(bits-1))
      end
    end

    def self.it_roundtrips_uint(bits, type)
      it type do
        roundtrip(type, 0)
        roundtrip(type, 1)
        roundtrip(type, 1234)
        roundtrip(type, 2**bits - 1)
      end
    end

    it_roundtrips_sint(32, :int32)
    it_roundtrips_sint(32, :sint32)
    it_roundtrips_sint(32, :sfixed32)
    it_roundtrips_uint(32, :uint32)
    it_roundtrips_uint(32, :fixed32)
    it_roundtrips_sint(64, :int64)
    it_roundtrips_sint(64, :sint64)
    it_roundtrips_sint(64, :sfixed64)
    it_roundtrips_uint(64, :uint64)
    it_roundtrips_uint(64, :fixed64)

    context 'allows field names to be' do
      it('symbols') { roundtrip(:string, 'sss', :foo) }
      it('string') { roundtrip(:string, 'sss', 'foo') }
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

      roundtrip(:string, 'hello, world!'.encode('utf-16')) do |v1, v2|
        expect(v1.encoding).to eql Encoding::UTF_16
        expect(v2.encoding).to eql Encoding::UTF_8
        expect(v2).to eql v1.encode('utf-8')
      end
    end

    it 'bytes' do
      roundtrip(:bytes, 'hello')
      roundtrip(:bytes, [0, 1, 2, 3, 255].pack('c*'))
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

    it 'bool' do
      roundtrip(:bool, true)
      roundtrip(:bool, false)
    end

    it 'embedded messages' do
      sub_msg = Pbr::TMessage.new('SubMsg', [Pbr::TField.new('bar', 1, Pbr::TFieldType::STRING, nil)])
      message_type = Pbr::TMessage.new('TestMsg', [Pbr::TField.new('foo', 1, Pbr::TFieldType::MESSAGE, sub_msg)])

      field_val = OpenStruct.new({bar: 'hello'})

      roundtrip_impl(message_type, field_val, 'foo') do |v1, v2|
        expect(v2).to be_a OpenStruct
        expect(v2.bar).to eql v1.bar
      end
    end


    def roundtrip(field_type_as_symbol, field_val, field_name=:foo, field_num=1, &block)
      field_type = "Pbr::TFieldType::#{field_type_as_symbol.to_s.upcase}".constantize
      message_type = Pbr::TMessage.new('TestMsg', [ Pbr::TField.new(field_name, field_num, field_type, nil) ])
      roundtrip_impl(message_type, field_val, field_name, &block)
    end

    def roundtrip_float(precision, field_type_as_symbol, field_val)
      roundtrip(field_type_as_symbol, field_val) do |v1, v2|
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
    end

    def roundtrip_impl(message_type, field_val, field_name, &block)
      block ||= ->(v1, v2){expect(v2).to eql v1}

      pbr = Pbr.new
      begin
        obj1 = OpenStruct.new
        obj1.send("#{field_name}=", field_val)

        bytes = pbr.write(obj1, message_type)
        obj2 = pbr.read(bytes, message_type)

        v1, v2 = [obj1, obj2].map{|obj| obj.send(field_name)}
        print_roundtrip(v1, v2)
        block.call(v1, v2)
      ensure
        pbr.close
      end
    end

    def print_roundtrip(v1, v2)
      shown = "#{v1.inspect}#{v1.is_a?(String) ? " (#{v1.encoding})" : ''} -> #{v2.inspect}#{v2.is_a?(String) ? " (#{v2.encoding})" : ''}"
      puts shown.size > 120 ? shown[0..100] + '...' : shown
    end
  end
end
