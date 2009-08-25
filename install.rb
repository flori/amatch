#!/usr/bin/env ruby

require 'rbconfig'
include Config
require 'fileutils'
include FileUtils::Verbose

MAKE = ENV['MAKE'] || %w[gmake make].find { |c| system(c, '-v') }

bindir  = CONFIG['bindir']
archdir = CONFIG['sitearchdir']
libdir  = CONFIG['sitelibdir']
dlext   = CONFIG['DLEXT']
cd 'ext' do
  system 'ruby extconf.rb' or exit 1
  system "#{MAKE}" or exit 1
  mkdir_p archdir
  install "amatch.#{dlext}", archdir
end
cd 'bin' do
  filename = 'edit_json.rb'
  install('agrep.rb', bindir)
end
cd 'lib/amatch' do
  mkdir_p d = File.join(libdir, 'amatch')
  install 'version.rb', d
end
warn " *** Installed amatch extension."
