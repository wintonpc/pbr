require 'rspec'
require 'pbr'

describe 'My behaviour' do
  it 'should do something' do
    pbr = Pbr.new(PbrRule.vanilla)
    req = pbr.read(File.read('spec/pb.dat'), CodeGeneratorRequest)
    pp req
  end
end
