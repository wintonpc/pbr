require 'rspec'
require 'pbr'

describe 'Message validation' do

  let!(:message_type) {
    Class.new do
      include Pbr::Message
      type_name 'ValidationTestMsg'

      Meal = Module.new do
        include Pbr::Enum
        BREAKFAST = 1
        LUNCH = 2
        DINNER = 3
      end

      required :req, :string, 1
      optional :opt, :string, 2
      repeated :rep, :string, 3

      optional :an_int, :int32, 4
      optional :a_bool, :bool, 5
      optional :an_enum, Meal, 6
    end
  }

  def self.it_validates_read(what, msg, err_type=Pbr::ValidationError, err_msg=nil)
    it "#{what} (read)" do
      pbr = Pbr.new(PbrMapping.hash_with_symbol_keys)
      crap = Pbr.new(PbrMapping.hash_with_symbol_keys, validate_on_write: false).write(msg, message_type)
      expect{pbr.read(crap, message_type)}.to raise_error(err_type, err_msg)
    end
  end

  def self.it_validates_write(what, msg, err_type=Pbr::ValidationError, err_msg=nil)
    it "#{what} (write)" do
      pbr = Pbr.new(PbrMapping.hash_with_symbol_keys)
      expect{pbr.write(msg, message_type)}.to raise_error(err_type, err_msg)
    end
  end

  def self.it_throws_on(what, msg, err_type, err_msg)
    it_validates_write(what, msg, err_type, err_msg)
  end

  it_validates_write('missing required fields', {opt: 'foo'})
  it_validates_read('missing required fields', {opt: 'foo'})
  it_validates_write('bad strings', {req: 555})
  it_throws_on('bad integers (string)', {req: '', an_int: 'hello'}, TypeError, 'no implicit conversion of String into Integer')
  it_validates_write('bad booleans', {req: '', a_bool: 0})
  it_validates_write('bad enums', {req: '', an_enum: 4})
  it_validates_read('bad enums', {req: '', an_enum: 4})
end
