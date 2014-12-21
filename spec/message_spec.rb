require 'rspec'
require 'pbr'
require 'ostruct'

class TestMsg
  include Pbr::Message

  required :foo, :string, 1
end

describe 'My behaviour' do

  it 'should do something' do
    pbr = Pbr.new(PbrRule.vanilla)
    x = TestMsg.new(foo: 'hello')
    bytes = pbr.write(x, TestMsg)
    y = pbr.read(bytes, TestMsg)

    expect(y).to be_a TestMsg
    expect(y.foo).to eql 'hello'
  end
end
