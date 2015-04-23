require 'rspec'
require 'pbr'
require 'ostruct'
require 'time'

class TestMsg; end
class SubMsg; end

class TestMsg
  include Pbr::Message
  required :foo, :string, 1
  repeated :words, :string, 2
  required :thing, SubMsg, 3
  repeated :children, SubMsg, 4
  required :stamp, :string, 5
  required :ranges, :string, 6

  deflate(:stamp) {|time| time.utc.iso8601(3)}
  inflate(:stamp) {|iso_str| Time.parse(iso_str).localtime}

  deflate(:ranges) {|r| r.to_s}
  inflate(:ranges) {|str| eval(str)}
end

class SubMsg
  include Pbr::Message
  required :bar, :int32, 1
end

class TinyMsg
  include Pbr::Message
  required :foo, :string, 1
  required :bar, :string, 2
end

class WrappedMsg
  include Pbr::Message
  required :type, :string, 1
  required :msg, :bytes, 2

  type_of(:msg) {|wrapped| wrapped.type.constantize }
end

describe Pbr do

  it 'constructing' do
    pbr = Pbr.new(PbrMapping.vanilla)
    bytes = pbr.write(make_valid_test_msg, TestMsg)
    y = pbr.read(bytes, TestMsg)

    expect(y).to be_a TestMsg
    expect(y.foo).to eql 'hello'
    expect(y.words).to eql ['ping', 'pong']
    expect(y.thing).to be_a SubMsg
    expect(y.thing.bar).to eql 55
    expect(y.children.size).to eql 2
    expect(y.children[0]).to be_a SubMsg
    expect(y.children[0].bar).to eql 66
    expect(y.children[1]).to be_a SubMsg
    expect(y.children[1].bar).to eql 77
    expect(y.stamp).to be_a Time
    expect(y.stamp.zone).to_not eql 'UTC'
    expect(y.ranges.size).to eql 2
    expect(y.ranges[0]).to be_a Range
    expect(y.ranges[0]).to eql (0..3)
    expect(y.ranges[1]).to be_a Range
    expect(y.ranges[1]).to eql (5..9)
  end

  it 'construct with bad field name' do
    expect{TestMsg.new(not_here: 5)}.to raise_error 'Cannot set nonexistent field TestMsg.not_here'
  end

  it 'allows custom field name mapping' do
    mapping = PbrMapping.new
    mapping.get_target_type = ->t{OpenStruct}
    mapping.get_target_field = ->f{f.name == :foo ? "x#{f.name}x" : f.name}

    obj = OpenStruct.new(xfoox: 'hello',
                         bar: 'goodbye')

    pbr = Pbr.new(mapping)
    bytes = pbr.write(obj, TinyMsg)
    y = pbr.read(bytes, TinyMsg)
    expect(y).to be_a OpenStruct
    expect(y.xfoox).to eql 'hello'
    expect(y.bar).to eql 'goodbye'
  end

  it 'type is optional when writing when using vanilla mapping' do
    m = make_valid_test_msg
    pbr = Pbr.new
    encoded = pbr.write(m)
    v2 = pbr.read(encoded, TestMsg)
    expect(v2).to be_a TestMsg

    expect{pbr.write({})}.to raise_error Pbr::Error
  end

  it 'use with hashes' do
    v1 = {'foo' =>      'hello',
          'words' =>    ['ping', 'pong'],
          'thing' =>    {'bar' => 55},
          'children' => [{'bar' => 66}, {'bar' => 77}],
          'stamp' =>    Time.now,
          'ranges' =>   [0..3, 5..9]}
    pbr = Pbr.new(PbrMapping.hash_with_string_keys)
    v2 = pbr.read(pbr.write(v1, TestMsg), TestMsg)
    expect(v2['foo']).to eql 'hello'
    expect(v2['stamp']).to be_a Time
  end

  it 'wrapped message' do
    pbr = Pbr.new
    v1 = WrappedMsg.new(type: 'TinyMsg', msg: TinyMsg.new(foo: 'hello', bar: 'goodbye'))
    expect{pbr.write(v1, WrappedMsg)}.to raise_error 'Lazy type TinyMsg for WrappedMsg.msg has not been registered.'

    pbr.register(TinyMsg)
    encoded = pbr.write(v1, WrappedMsg)
    v2 = pbr.read(encoded, WrappedMsg)
    expect(v2.msg).to be_a TinyMsg
    expect(v2.msg.foo).to eql 'hello'
    expect(v2.msg.bar).to eql 'goodbye'
  end

  # it 'regression' do
  #   pbr = Pbr.new
  #   pbr.write(make_valid_test_msg)
  #   pbr.write(TinyMsg.new(foo: '', bar: ''))
  #   pbr.write(make_valid_test_msg)
  # end

  def make_valid_test_msg
    TestMsg.new(foo:      'hello',
                words:    ['ping', 'pong'],
                thing:    {bar: 55},
                children: [{bar: 66}, {bar: 77}],
                stamp:    Time.now,
                ranges:   [0..3, 5..9])
  end
end
