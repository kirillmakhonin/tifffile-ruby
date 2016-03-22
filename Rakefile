require "rake/extensiontask"
require 'rspec/core'
require 'rspec/core/rake_task'


Rake::ExtensionTask.new "tifffile" do |ext|
  ext.lib_dir = "lib/tifffile"
end

RSpec::Core::RakeTask.new(:spec) do |spec|
  Rake::Task['compile'].invoke
  spec.pattern = FileList['spec/**/*_spec.rb']
end

