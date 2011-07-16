# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name = %q{amatch}
  s.version = "0.2.6"

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = ["Florian Frank"]
  s.date = %q{2011-07-16}
  s.default_executable = %q{agrep.rb}
  s.description = %q{Library to tail files in Ruby}
  s.email = %q{flori@ping.de}
  s.executables = ["agrep.rb"]
  s.extensions = ["ext/extconf.rb"]
  s.extra_rdoc_files = ["README.rdoc", "lib/amatch/version.rb", "ext/amatch.c", "ext/pair.c"]
  s.files = [".gitignore", "CHANGES", "COPYING", "Gemfile", "README.rdoc", "Rakefile", "VERSION", "amatch.gemspec", "bin/agrep.rb", "ext/amatch.c", "ext/common.h", "ext/extconf.rb", "ext/pair.c", "ext/pair.h", "install.rb", "lib/amatch/.keep", "lib/amatch/version.rb", "tests/test_hamming.rb", "tests/test_jaro.rb", "tests/test_jaro_winkler.rb", "tests/test_levenshtein.rb", "tests/test_longest_subsequence.rb", "tests/test_longest_substring.rb", "tests/test_pair_distance.rb", "tests/test_sellers.rb"]
  s.homepage = %q{http://github.com/flori/amatch}
  s.rdoc_options = ["--title", "Amatch - Approximate Matching", "--main", "README.rdoc"]
  s.require_paths = ["lib", "ext"]
  s.rubygems_version = %q{1.6.2}
  s.summary = %q{Approximate String Matching library}
  s.test_files = ["tests/test_sellers.rb", "tests/test_jaro.rb", "tests/test_longest_subsequence.rb", "tests/test_longest_substring.rb", "tests/test_hamming.rb", "tests/test_pair_distance.rb", "tests/test_levenshtein.rb", "tests/test_jaro_winkler.rb"]

  if s.respond_to? :specification_version then
    s.specification_version = 3

    if Gem::Version.new(Gem::VERSION) >= Gem::Version.new('1.2.0') then
      s.add_development_dependency(%q<gem_hadar>, ["~> 0.0.3"])
      s.add_runtime_dependency(%q<spruz>, ["~> 0.2"])
    else
      s.add_dependency(%q<gem_hadar>, ["~> 0.0.3"])
      s.add_dependency(%q<spruz>, ["~> 0.2"])
    end
  else
    s.add_dependency(%q<gem_hadar>, ["~> 0.0.3"])
    s.add_dependency(%q<spruz>, ["~> 0.2"])
  end
end
