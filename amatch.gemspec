    Gem::Specification.new do |s|
      s.name = 'amatch'
      s.version = '0.2.5'
      s.summary = "Approximate String Matching library"
      s.description = <<EOF
Amatch is a library for approximate string matching and searching in strings.
Several algorithms can be used to do this, and it's also possible to compute a
similarity metric number between 0.0 and 1.0 for two given strings.
EOF

      s.files = ["CHANGES", "COPYING", "README", "Rakefile", "VERSION", "amatch.gemspec", "bin", "bin/agrep.rb", "ext", "ext/amatch.c", "ext/common.h", "ext/extconf.rb", "ext/pair.c", "ext/pair.h", "install.rb", "lib", "lib/amatch", "lib/amatch.so", "lib/amatch/version.rb", "tests", "tests/test_hamming.rb", "tests/test_jaro.rb", "tests/test_jaro_winkler.rb", "tests/test_levenshtein.rb", "tests/test_longest_subsequence.rb", "tests/test_longest_substring.rb", "tests/test_pair_distance.rb", "tests/test_sellers.rb"]

      s.extensions << "ext/extconf.rb"

      s.require_paths << 'ext' << 'lib'

      s.bindir = "bin"
      s.executables = ["agrep.rb"]
      s.default_executable = "agrep.rb"

      s.has_rdoc = true
      s.extra_rdoc_files.concat ["doc-main.txt", "ext/amatch.c", "lib/amatch/version.rb"]
      s.rdoc_options << '--main' << 'doc-main.txt' <<
        '--title' << "amatch - Approximate Matching"
      s.test_files.concat Dir['tests/test_*.rb']

      s.author = "Florian Frank"
      s.email = "flori@ping.de"
      s.homepage = "http://amatch.rubyforge.org"
      s.rubyforge_project = 'amatch'
    end
