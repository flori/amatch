# vim: set filetype=ruby et sw=2 ts=2:

begin
  require 'rake/gempackagetask'
rescue LoadError
end
require 'rbconfig'
include Config
require 'rake/clean'
CLEAN.include 'coverage', 'doc'
require 'rake/testtask'

MAKE        = ENV['MAKE'] || %w[gmake make].find { |c| system(c, '-v') }
PKG_NAME    = 'amatch'
PKG_VERSION = File.read('VERSION').chomp
PKG_FILES   = FileList["**/*"].exclude(/^(pkg|coverage|doc)/)
PKG_DOC_FILES = [ "ext/amatch.c" ].concat(Dir['lib/**/*.rb']) << 'doc-main.txt'

task :default => :test

desc "Run unit tests"
task :test => :compile_ext do
  sh %{testrb -Iext:lib tests/test_*.rb}
end

desc "Compiling library"
task :compile_ext do
  cd 'ext'  do
    ruby %{extconf.rb}
    sh MAKE
  end
end

desc "Installing library"
task :install => :test do
  src, = Dir['ext/amatch.*'].reject { |x| /\.[co]$/.match x }
  filename = File.basename(src)
  dst = File.join(CONFIG["sitelibdir"], filename)
  install(src, dst, :verbose => true)
end

desc "Removing generated files"
task :clean do
  cd 'ext' do
    ruby 'extconf.rb'
    sh "#{MAKE} distclean" if File.exist?('Makefile')
  end
end

desc "Build the documentation"
task :doc do
  sh "rdoc -m doc-main.txt -t '#{PKG_NAME} - Approximate Matching' #{PKG_DOC_FILES * ' '}"
end

if defined? Gem
  spec = Gem::Specification.new do |s|
    s.name = 'amatch'
    s.version = PKG_VERSION
    s.summary = "Approximate String Matching library"
    s.description = <<EOF
Amatch is a library for approximate string matching and searching in strings.
Several algorithms can be used to do this, and it's also possible to compute a
similarity metric number between 0.0 and 1.0 for two given strings.
EOF

    s.files = PKG_FILES

    s.extensions << "ext/extconf.rb"

    s.require_path = 'ext'

    s.bindir = "bin"
    s.executables = ["agrep.rb"]
    s.default_executable = "agrep.rb"

    s.has_rdoc = true
    s.extra_rdoc_files.concat PKG_DOC_FILES
    s.rdoc_options << '--main' << 'doc-main.txt' <<
      '--title' << "#{PKG_NAME} - Approximate Matching"
    s.test_files.concat Dir['tests/test_*.rb']

    s.author = "Florian Frank"
    s.email = "flori@ping.de"
    s.homepage = "http://amatch.rubyforge.org"
    s.rubyforge_project = "amatch"
  end

  Rake::GemPackageTask.new(spec) do |pkg|
    pkg.need_tar      = true
    pkg.package_files += PKG_FILES
  end
end

desc m = "Writing version information for #{PKG_VERSION}"
task :version do
  puts m
  File.open(File.join('lib', 'amatch', 'version.rb'), 'w') do |v|
    v.puts <<EOT
module Amatch
  # Amatch version
  VERSION         = '#{PKG_VERSION}'
  VERSION_ARRAY   = VERSION.split(/\\./).map { |x| x.to_i } # :nodoc:
  VERSION_MAJOR   = VERSION_ARRAY[0] # :nodoc:
  VERSION_MINOR   = VERSION_ARRAY[1] # :nodoc:
  VERSION_BUILD   = VERSION_ARRAY[2] # :nodoc:
end
EOT
  end
end


desc "Prepare a new release"
task :release => [ :clean, :version, :package ]
