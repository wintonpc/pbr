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

  def self.it_validates(what, msg)
    pbr = Pbr.new(PbrMapping.hash_with_symbol_keys)
    it "#{what} (write)" do
      expect{pbr.write(msg, message_type)}.to raise_error Pbr::ValidationError
    end
    it "#{what} (read)" do
      crap = Pbr.new(PbrMapping.hash_with_symbol_keys, validate_on_write: false).write(msg, message_type)
      expect{pbr.read(crap, message_type)}.to raise_error Pbr::ValidationError
    end
  end

  it_validates('missing required fields', {opt: 'foo'})
end
