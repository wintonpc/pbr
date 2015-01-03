# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'pbr/version'

Gem::Specification.new do |spec|
  spec.name          = 'pbr'
  spec.version       = Pbr::VERSION
  spec.authors       = ['Peter Winton']
  spec.email         = ['pwinton@indigobio.com']
  spec.summary       = %q{Fast ProtoBuf}
  spec.description   = %q{Fast ProtoBuf}
  spec.homepage      = ''
  spec.license       = 'MIT'

  spec.files         = `git ls-files -z`.split("\x0")
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ['lib']

  spec.extensions << 'ext/pbr_ext/extconf.rb'

  spec.add_development_dependency 'bundler', '~> 1.7'
  spec.add_development_dependency 'rake', '~> 10.0'
  spec.add_development_dependency 'rspec'
  spec.add_development_dependency 'rake-compiler'
  spec.add_development_dependency 'oj'
  spec.add_runtime_dependency 'activesupport'

end
