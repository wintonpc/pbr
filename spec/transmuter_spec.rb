require 'rspec'
require 'pbr'
require 'ostruct'

describe Pbr::Transmuter do

  class FooMsg
    include Pbr::Message
    required :a, :int32, 1
    required :b, :int32, 2
  end

  describe '#transmute' do
    it 'converts an object between different types' do
      tx = Pbr::Transmuter.new(PbrMapping.hash_with_symbol_keys, PbrMapping.vanilla)
      v1 = {a: 3, b: 4}
      v2 = tx.transmute(v1, FooMsg)
      expect(v2).to be_a FooMsg
      expect(v2.a).to eql 3
      expect(v2.b).to eql 4
    end
  end
end
