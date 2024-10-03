# Changes

## 2022-05-15 v0.4.1

* **Moved CI testing from Travis to All Images**
  + Updated configuration to use `all_images` instead of Travis for continuous integration testing.

## 2017-07-04 v0.4.0

  * Officially support DamerauLevenshtein matching algorithm.
  * Change license to Apache 2.0

## 2017-05-23 v0.3.1

  * Include PairDistance fix from dominikgrygiel, Thx.

## 2014-03-27 v0.3.0

  * Update some dependencies

## 2013-10-14 v0.2.12

  * Include test fix from Juanito Fatas <katehuang0320@gmail.com>. Thx!

## 2013-01-16 v0.2.11

  * Include some fixes from Jason Colburne <jason@redbeardenterprises.com>.
    Thx!

## 2012-02-06 v0.2.10

  * Use xfree instead of free to avoid (possible) problems.

## 2011-11-15 v0.2.9

  * Provide amatch/rude and amatch/polite for require (the latter doesn't
    extend ::String on its own)
  * `pair_distance_similar` method now can take an optional regexp argument for
    tokenizing.

## 2011-08-06 v0.2.8

  * Depend on tins library.

## 2011-08-06 v0.2.7

  * Fix some violations of ISO C90 standard.

## 2011-07-16 v0.2.6

  * Applied patch by Kevin J. Lynagh <kevin@keminglabs.com> fixing memory
    leak in Jaro match.

## 2009-09-25 v0.2.5

  * Added lib to gem's require_paths.
  * Using rake-compiler now.

## 2009-08-25 v0.2.4

  * Included Jaro and Jaro-Winkler metrics implementation of Kevin Ballard
    <kevin@rapleaf.com>. Thanks a lot.
  * Made the extension compile under Ruby 1.9.

## 2006-06-25 v0.2.3

  * Fixed agrep.rb to use the new API.

## 2005-10-11 v0.2.2
  * Fixed a typo in extconf.rb that prohibitted compiling on
    non-gcc compilers.

## 2005-09-12 v0.2.1

 * Bugfix: Wrong type for pattern length corrected. Thanks to David
   Heinemeier Hansson for reporting it.

## 2005-06-01 v0.2.0

  * Major changes in API and implementation:
    Now the Levenshtein edit distance, Sellers edit distance, the Hamming
    distance, the longest common subsequence length, the longest common
    substring length, and the pair distance metric can be computed.

## 2005-01-20 v0.1.4

  * Better argument handling in initialization method
  * Minor changes in Rakefile and README.en

## 2004-09-27 v0.1.3

  * Rakefile and gem support added.

## 2004-09-24 v0.1.2

  * Uses Test::Unit for regression tests now.

## 2002-04-21 v0.1.1

  * Minor changes: documentation, more test cases and exceptions.

## 2009-08-26 v0.1.0

  * Initial Version
