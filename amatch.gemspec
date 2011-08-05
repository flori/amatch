# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name = %q{amatch}
  s.version = "0.2.7"

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = [%q{Florian Frank}]
  s.date = %q{2011-08-05}
  s.description = %q{Library to tail files in Ruby}
  s.email = %q{flori@ping.de}
  s.executables = [%q{agrep.rb}]
  s.extensions = [%q{ext/extconf.rb}]
  s.extra_rdoc_files = [%q{README.rdoc}, %q{lib/amatch/version.rb}, %q{ext/amatch.c}, %q{ext/pair.c}]
  s.files = [%q{.gitignore}, %q{.travis.yml}, %q{CHANGES}, %q{COPYING}, %q{Gemfile}, %q{README.rdoc}, %q{Rakefile}, %q{VERSION}, %q{amatch.gemspec}, %q{bin/agrep.rb}, %q{ext/amatch.c}, %q{ext/common.h}, %q{ext/extconf.rb}, %q{ext/pair.c}, %q{ext/pair.h}, %q{install.rb}, %q{lib/amatch/.keep}, %q{lib/amatch/version.rb}, %q{tests/test_hamming.rb}, %q{tests/test_jaro.rb}, %q{tests/test_jaro_winkler.rb}, %q{tests/test_levenshtein.rb}, %q{tests/test_longest_subsequence.rb}, %q{tests/test_longest_substring.rb}, %q{tests/test_pair_distance.rb}, %q{tests/test_sellers.rb}]
  s.homepage = %q{http://github.com/flori/amatch}
  s.rdoc_options = [%q{--title}, %q{Amatch - Approximate Matching}, %q{--main}, %q{README.rdoc}]
  s.require_paths = [%q{lib}, %q{ext}]
  s.rubygems_version = %q{1.8.7}
  s.summary = %q{Approximate String Matching library}
  s.test_files = [%q{tests/test_sellers.rb}, %q{tests/test_jaro.rb}, %q{tests/test_longest_subsequence.rb}, %q{tests/test_longest_substring.rb}, %q{tests/test_hamming.rb}, %q{tests/test_pair_distance.rb}, %q{tests/test_levenshtein.rb}, %q{tests/test_jaro_winkler.rb}]

  if s.respond_to? :specification_version then
    s.specification_version = 3

    if Gem::Version.new(Gem::VERSION) >= Gem::Version.new('1.2.0') then
      s.add_development_dependency(%q<gem_hadar>, ["~> 0.0.11"])
      s.add_runtime_dependency(%q<spruz>, ["~> 0.2"])
    else
      s.add_dependency(%q<gem_hadar>, ["~> 0.0.11"])
      s.add_dependency(%q<spruz>, ["~> 0.2"])
    end
  else
    s.add_dependency(%q<gem_hadar>, ["~> 0.0.11"])
    s.add_dependency(%q<spruz>, ["~> 0.2"])
  end
end
