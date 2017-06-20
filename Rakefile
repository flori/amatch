# vim: set filetype=ruby et sw=2 ts=2:

require 'gem_hadar'

GemHadar do
  name        'amatch'
  author      'Florian Frank'
  email       'flori@ping.de'
  homepage    "http://github.com/flori/#{name}"
  summary     'Approximate String Matching library'
  description <<EOT
Amatch is a library for approximate string matching and searching in strings.
Several algorithms can be used to do this, and it's also possible to compute a
similarity metric number between 0.0 and 1.0 for two given strings.
EOT
  executables << 'agrep' << 'dupfind'
  bindir      'bin'
  test_dir    'tests'
  ignore      '.*.sw[pon]', 'pkg', 'Gemfile.lock', '.AppleDouble', '.rbx', 'Makefile'
  title       "#{name.camelize} - Approximate Matching"
  readme      'README.rdoc'
  require_paths %w[lib ext]
  dependency             'tins',      '~>1.0'
  dependency             'infobar'
  dependency             'mize'
  development_dependency 'test-unit', '~>3.0'
  licenses << 'GPL'
end
