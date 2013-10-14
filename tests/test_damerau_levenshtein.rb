require 'test/unit'
require 'amatch'

class TestDamerauLevenshtein < Test::Unit::TestCase
  include Amatch

  def setup
    @d        = 0.000001
    @empty    = DamerauLevenshtein.new('')
    @simple   = DamerauLevenshtein.new('test')
    @long     = DamerauLevenshtein.new('A' * 160)
  end

  def test_version
    assert_kind_of String, Amatch::VERSION
  end

  def test_match
    assert_equal 4,     @simple.match('')
    assert_equal 0,     @simple.match('test')
    assert_equal 1,     @simple.match('testa')
    assert_equal 1,     @simple.match('atest')
    assert_equal 1,     @simple.match('teast')
    assert_equal 1,     @simple.match('est')
    assert_equal 1,     @simple.match('tes')
    assert_equal 1,     @simple.match('tst')
    assert_equal 1,     @simple.match('best')
    assert_equal 1,     @simple.match('tost')
    assert_equal 1,     @simple.match('tesa')
    assert_equal 3,     @simple.match('taex')
    assert_equal 6,     @simple.match('aaatestbbb')
    assert_equal 1,     @simple.match('tset')
  end

  def test_search
    assert_equal 4,     @simple.search('')
    assert_equal 0,     @empty.search('')
    assert_equal 0,     @empty.search('test')
    assert_equal 0,     @simple.search('aaatestbbb')
    assert_equal 3,     @simple.search('aaataexbbb')
    assert_equal 4,     @simple.search('aaaaaaaaa')
  end

  def test_array_result
    assert_equal [1, 0],    @simple.match(["tets", "test"])
    assert_equal [1, 0],    @simple.search(["tetsaaa", "testaaa"])
    assert_raises(TypeError) { @simple.match([:foo, "bar"]) }
  end

  def test_pattern_setting
    assert_raises(TypeError) { @simple.pattern = :something }
    assert_equal 0, @simple.match('test')
    @simple.pattern = ''
    assert_equal 4, @simple.match('test')
    @simple.pattern = 'test'
    assert_equal 0, @simple.match('test')
  end

  def test_similar
    assert_in_delta 1, @empty.similar(''), @d
    assert_in_delta 0, @empty.similar('not empty'), @d
    assert_in_delta 0.0, @simple.similar(''), @d
    assert_in_delta 1.0, @simple.similar('test'), @d
    assert_in_delta 0.8, @simple.similar('testa'), @d
    assert_in_delta 0.8, @simple.similar('atest'), @d
    assert_in_delta 0.8, @simple.similar('teast'), @d
    assert_in_delta 0.75, @simple.similar('est'), @d
    assert_in_delta 0.75, @simple.similar('tes'), @d
    assert_in_delta 0.75, @simple.similar('tst'), @d
    assert_in_delta 0.75, @simple.similar('best'), @d
    assert_in_delta 0.75, @simple.similar('tost'), @d
    assert_in_delta 0.75, @simple.similar('tesa'), @d
    assert_in_delta 0.25, @simple.similar('taex'), @d
    assert_in_delta 0.4, @simple.similar('aaatestbbb'), @d
    assert_in_delta 0.75, @simple.pattern.damerau_levenshtein_similar('est'), @d
  end

  def test_transpositions
    assert_in_delta 1.0, 'atestatest'.damerau_levenshtein_similar('atestatest'), @d
    assert_in_delta 0.9, 'atestatest'.damerau_levenshtein_similar('atetsatest'), @d
    assert_in_delta 0.8, 'atestatest'.damerau_levenshtein_similar('atetsatset'), @d
  end

  def test_long
    assert_in_delta 1.0, @long.similar(@long.pattern), @d
  end

  def test_long2
    a = "lost this fantasy,  this fantasy,  this  fantasy,  this fantasy,  this fantasy,  this fantasy\r\n\r\nGood love  Neat work\r\n\r\nSuper job Fancy work\r\n\r\nPants job Cool work"
    b = "lost\r\n\r\nGood love Neat work\r\n\r\nSuper  job Fancy work\r\n\r\nPants job Cool work"
    assert a.damerau_levenshtein_similar(b)
  end
end
