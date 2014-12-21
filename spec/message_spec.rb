require 'rspec'
require 'pbr'
require 'ostruct'

class TestMsg; end
class SubMsg; end

class TestMsg
  include Pbr::Message
  required :foo, :string, 1
  repeated :words, :string, 2
  required :thing, SubMsg, 3
  repeated :children, SubMsg, 4
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
  end
end
