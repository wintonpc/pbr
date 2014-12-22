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
  # required :stamp, :string, 5

  # deflate(:stamp) {|time| time.utc.iso8601(3)}
  # inflate(:stamp) {|iso_str| Time.parse(iso_str).localtime}
end

class SubMsg
  include Pbr::Message
  required :bar, :int32, 1
end

describe 'My behaviour' do

  it 'constructing' do
    pbr = Pbr.new(PbrRule.vanilla)
    x = TestMsg.new(foo: 'hello',
                    words: ['ping', 'pong'],
                    thing: {bar: 55},
                    children: [{bar: 66}, {bar: 77}])
                    # stamp: Time.now)

    bytes = pbr.write(x, TestMsg)
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
    # expect(y.stamp).to be_a Time
    # expect(y.stamp.zone).to_not eql 'UTC'
  end

  it 'construct with bad field name' do
    expect{TestMsg.new(not_here: 5)}.to raise_error 'Cannot set nonexistent field TestMsg.not_here'
  end
end
