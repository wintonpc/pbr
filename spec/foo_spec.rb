require 'rspec'
require 'pbr'

describe 'My behaviour' do
  it 'should do something' do
    pbr = Pbr.new(PbrRule.vanilla)
    ns = ''
    req = pbr.read(File.read('spec/pb.dat'), CodeGeneratorRequest)
    compiled = Pbr::Generator.compile(ns, req)
    res = pbr.write(compiled, CodeGeneratorResponse)
    STDOUT.print(res)
  end
end
