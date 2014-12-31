require 'rspec'
require 'pbr'

describe 'Descriptor validation' do

  it 'validates field names' do
    field_args = [:required, 'foo', :int32, 1]
    expect_error('Invalid name "foo", must be symbol', field_args)
  end

  it 'validates labels' do
    field_args = [:rekwired, :foo, :int32, 1]
    expect_error('Invalid label: rekwired', field_args)
  end

  it 'forbids fields with duplicate names' do
    expect_error('A field named foo is already declared',
                 [:required, :foo, :int32, 1],
                 [:required, :foo, :int32, 2])
  end

  it 'forbids fields with duplicate numbers' do
    expect_error('A field with number 5 is already declared',
                 [:required, :foo, :int32, 5],
                 [:required, :bar, :int32, 5])
  end

  it 'validates field types' do
    expect_error('Invalid field type: 99', [:required, :foo, 99, 1])
    expect_error("'FieldDescriptorProto::Type::INT33' has not been defined as a constant", [:required, :foo, :int33, 1])
  end

  def expect_error(expected_msg, *multiple_field_args)
    expect {
      Class.new do
        include Pbr::Message
        multiple_field_args.each{|field_args| field(*field_args)}
      end
    }.to raise_error Pbr::DescriptorError, expected_msg
  end

end
