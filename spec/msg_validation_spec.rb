require 'rspec'
require 'pbr'

describe 'Message validation' do

  let!(:message_type) {
    Class.new do
      include Pbr::Message
      type_name 'ValidationTestMsg'

      required :req, :string, 1
      optional :opt, :string, 2
      repeated :rep, :string, 3
    end
  }

  def self.it_validates_read(what, msg)
    it "#{what} (read)" do
      pbr = Pbr.new(PbrMapping.hash_with_symbol_keys)
      crap = Pbr.new(PbrMapping.hash_with_symbol_keys, validate_on_write: false).write(msg, message_type)
      expect{pbr.read(crap, message_type)}.to raise_error Pbr::ValidationError
    end
  end

  def self.it_validates_write(what, msg)
    it "#{what} (write)" do
      pbr = Pbr.new(PbrMapping.hash_with_symbol_keys)
      expect{pbr.write(msg, message_type)}.to raise_error Pbr::ValidationError
    end
  end

  it_validates_write('missing required fields', {opt: 'foo'})
  it_validates_read('missing required fields', {opt: 'foo'})
  it_validates_write('bad strings', {req: 555})
end
