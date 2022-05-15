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
  package_ignore '.all_images.yml', '.gitignore', 'VERSION'
  title       "#{name.camelize} - Approximate Matching"
  readme      'README.md'
  require_paths %w[lib ext]
  dependency             'tins',      '~>1.0'
  dependency             'mize'
  development_dependency 'test-unit', '~>3.0'
  development_dependency 'all_images'
  required_ruby_version '>=2.4'
  licenses << 'Apache-2.0'
end
