# amatch - Approximate Matching Extension for Ruby 📏

## Description 📝

`amatch` is a high-performance collection of classes used for approximate
matching, searching, and comparing strings. It provides an efficient Ruby
interface to several industry-standard algorithms for calculating edit distance
and string similarity.

## Supported Algorithms 🧩

The library implements a wide array of metrics to suit different matching needs:

*   **Levenshtein Distance**: The classic "edit distance" (insertions,
    deletions, substitutions).
*   **Sellers Algorithm**: A variation of Levenshtein optimized for searching a
    pattern within a longer text.
*   **Damerau-Levenshtein**: Similar to Levenshtein but considers
    transpositions of two adjacent characters as a single edit.
*   **Hamming Distance**: Measures the number of positions at which
    corresponding symbols are different (only for strings of equal length).
*   **Jaro-Winkler**: A metric geared towards short strings like names, giving
    more weight to prefix matches.
*   **Pair Distance**: A flexible distance metric based on character pairs
    (also available as `Amatch::DiceCoefficient`). Unlike set-based measures,
    this implementation uses multisets, meaning it is sensitive to the frequency
    of repeated character pairs.
*   **Longest Common Subsequence/Substring**: Finds the longest shared
    sequences between two strings.

## Installation 📦

You can install the extension as a gem:

```shell
gem install amatch
```

Alternatively, if you prefer manual installation:

```shell
ruby install.rb
# or
rake install
```

## Usage 🛠️

### Basic Setup

To get started, simply require the library and include the `Amatch` module to
add similarity methods directly to the `String` class.

```ruby
require 'amatch'
include Amatch
```

### Edit Distance Algorithms 📉

These algorithms return the "cost" to transform one string into another. Lower
values indicate higher similarity.

#### Levenshtein & Damerau-Levenshtein

```ruby
# Standard Levenshtein
m = Levenshtein.new("pattern")
m.match("pattren") # => 2
"pattern language".levenshtein_similar("language of patterns") # => 0.2

# Damerau-Levenshtein (handles transpositions)
m = Amatch::DamerauLevenshtein.new("pattern")
m.match("pattren") # => 1
"pattern language".damerau_levenshtein_similar("language of patterns") # => 0.2
```

#### Sellers (Pattern Searching)

Sellers is particularly useful for finding the best match of a pattern within a
larger body of text.

```ruby
m = Sellers.new("pattern")
m.match("pattren") # => 2.0

# You can customize weights for different edit types
m.substitution = m.insertion = 3
m.match("pattren") # => 4.0

m.reset_weights
m.search("abcpattrendef") # => 2.0
```

#### Hamming Distance

Used primarily for strings of equal length to count substitutions.

```ruby
m = Hamming.new("pattern")
m.match("pattren") # => 2
"pattern language".hamming_similar("language of patterns") # => 0.1
```

### Similarity Metrics 📈

These algorithms typically return a score between `0.0` and `1.0`, where `1.0`
is a perfect match.

#### Jaro-Winkler

Highly effective for record linkage and matching names.

```ruby
m = JaroWinkler.new("pattern")
m.match("paTTren") # => 0.9714...
m.ignore_case = false
m.match("paTTren") # => 0.7942...

# Custom scaling factor for prefix bonus
m.scaling_factor = 0.05
m.match("pattren") # => 0.9619...

"pattern language".jarowinkler_similar("language of patterns") # => 0.6722...
```

#### Jaro

The base metric for the Winkler variation.

```ruby
m = Jaro.new("pattern")
m.match("paTTren") # => 0.9523...
"pattern language".jaro_similar("language of patterns") # => 0.6722...
```

#### Other Metrics (Pair Distance, LCS, Longest Substring)

```ruby
# Pair Distance
# Note: This implementation uses multisets, meaning it considers character 
# frequencies rather than just unique pairs.
m = PairDistance.new("pattern")
m.match("pattr en") # => 0.5454...

# Pro Tip: Pass a regex as the second argument to match based on tokens
#  (e.g., words) rather than individual characters. This is particularly
# useful for natural language.
m.match("language of patterns", /\s+/)
"pattern language".pair_distance_similar("language of patterns", /\s+/) # => 0.9285...

# Longest Common Subsequence
m = LongestSubsequence.new("pattern")
m.match("pattren") # => 6
"pattern language".longest_subsequence_similar("language of patterns") # => 0.4

# Longest Common Substring
m = LongestSubstring.new("pattern")
m.match("pattren") # => 4
"pattern language".longest_substring_similar("language of patterns") # => 0.4
```

## Performance ⚡

`amatch` is implemented as a C extension to ensure maximum throughput when
processing large datasets or complex string comparisons.

![performance](http://cs304915.userapi.com/v304915401/5c97/BAzazF5E4Fo.jpg)

## Download 📥

The homepage of this library is located at:
* [https://github.com/flori/amatch](https://github.com/flori/amatch)

## Author 👨‍💻

[Florian Frank](mailto:flori@ping.de)

## License 📄

Apache License, Version 2.0 – See the COPYING file in the source archive.
