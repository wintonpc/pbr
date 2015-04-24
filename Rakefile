require 'rake/extensiontask'
require 'bundler/gem_tasks'

Rake::ExtensionTask.new('pbr_ext')

begin
  require 'rspec/core/rake_task'

  RSpec::Core::RakeTask.new(:spec)

  task :default => :spec
rescue LoadError
  # no rspec available
end
