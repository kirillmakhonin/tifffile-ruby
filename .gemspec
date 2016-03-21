require File.expand_path("../lib/tifffile/version", __FILE__)

Gem::Specification.new do |gem|
  gem.name    = 'tifffile'
  gem.version = TiffFile::VERSION
  gem.date    = '2016-03-21'

  gem.summary = "TIFF reader and writer"
  gem.description = ""

  gem.licenses = ['MIT']

  gem.authors  = ['Kirill Makhonin']
  gem.email    = 'kroks.rus@gmail.com'
  gem.homepage = 'https://github.com/kirillmakhonin/tifffile-ruby'


  gem.files    = `git ls-files`.split($\)

  
end


