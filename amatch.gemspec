# -*- encoding: utf-8 -*-
# stub: amatch 0.4.1 ruby lib ext
# stub: ext/extconf.rb

Gem::Specification.new do |s|
  s.name = "amatch".freeze
  s.version = "0.4.1".freeze

  s.required_rubygems_version = Gem::Requirement.new(">= 0".freeze) if s.respond_to? :required_rubygems_version=
  s.require_paths = ["lib".freeze, "ext".freeze]
  s.authors = ["Florian Frank".freeze]
  s.date = "2024-08-31"
  s.description = "Amatch is a library for approximate string matching and searching in strings.\nSeveral algorithms can be used to do this, and it's also possible to compute a\nsimilarity metric number between 0.0 and 1.0 for two given strings.\n".freeze
  s.email = "flori@ping.de".freeze
  s.executables = ["agrep".freeze, "dupfind".freeze]
  s.extensions = ["ext/extconf.rb".freeze]
  s.extra_rdoc_files = ["README.md".freeze, "lib/amatch.rb".freeze, "lib/amatch/polite.rb".freeze, "lib/amatch/rude.rb".freeze, "lib/amatch/version.rb".freeze, "ext/amatch_ext.c".freeze, "ext/pair.c".freeze]
  s.files = ["CHANGES.md".freeze, "COPYING".freeze, "Gemfile".freeze, "README.md".freeze, "Rakefile".freeze, "amatch.gemspec".freeze, "bin/agrep".freeze, "bin/dupfind".freeze, "ext/amatch_ext.c".freeze, "ext/common.h".freeze, "ext/extconf.rb".freeze, "ext/pair.c".freeze, "ext/pair.h".freeze, "images/amatch_ext.png".freeze, "install.rb".freeze, "lib/amatch.rb".freeze, "lib/amatch/.keep".freeze, "lib/amatch/polite.rb".freeze, "lib/amatch/rude.rb".freeze, "lib/amatch/version.rb".freeze, "tests/test_damerau_levenshtein.rb".freeze, "tests/test_hamming.rb".freeze, "tests/test_jaro.rb".freeze, "tests/test_jaro_winkler.rb".freeze, "tests/test_levenshtein.rb".freeze, "tests/test_longest_subsequence.rb".freeze, "tests/test_longest_substring.rb".freeze, "tests/test_pair_distance.rb".freeze, "tests/test_sellers.rb".freeze]
  s.homepage = "http://github.com/flori/amatch".freeze
  s.licenses = ["Apache-2.0".freeze]
  s.rdoc_options = ["--title".freeze, "Amatch - Approximate Matching".freeze, "--main".freeze, "README.md".freeze]
  s.required_ruby_version = Gem::Requirement.new(">= 2.4".freeze)
  s.rubygems_version = "3.5.16".freeze
  s.summary = "Approximate String Matching library".freeze
  s.test_files = ["tests/test_damerau_levenshtein.rb".freeze, "tests/test_hamming.rb".freeze, "tests/test_jaro.rb".freeze, "tests/test_jaro_winkler.rb".freeze, "tests/test_levenshtein.rb".freeze, "tests/test_longest_subsequence.rb".freeze, "tests/test_longest_substring.rb".freeze, "tests/test_pair_distance.rb".freeze, "tests/test_sellers.rb".freeze]

  s.specification_version = 4

  s.add_development_dependency(%q<gem_hadar>.freeze, ["~> 1.17.0".freeze])
  s.add_development_dependency(%q<test-unit>.freeze, ["~> 3.0".freeze])
  s.add_development_dependency(%q<all_images>.freeze, [">= 0".freeze])
  s.add_runtime_dependency(%q<tins>.freeze, ["~> 1.0".freeze])
  s.add_runtime_dependency(%q<mize>.freeze, [">= 0".freeze])
end
