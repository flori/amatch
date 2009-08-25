require 'test/unit'
require 'amatch'

class TestLongestSubsequence < Test::Unit::TestCase
  include Amatch

  D = 0.000001

  def setup
    @small   = LongestSubsequence.new('test')
    @empty   = LongestSubsequence.new('')
    @long    = LongestSubsequence.new('A' * 160)
  end

  def test_empty_subsequence
    assert_equal 0, @empty.match('')
    assert_equal 0, @empty.match('a')
    assert_equal 0, @small.match('')
    assert_equal 0, @empty.match('not empty')
  end

  def test_small_subsequence
    assert_equal 4, @small.match('test')
    assert_equal 4, @small.match('testa')
    assert_equal 4, @small.match('atest')
    assert_equal 4, @small.match('teast')
    assert_equal 3, @small.match('est')
    assert_equal 3, @small.match('tes')
    assert_equal 3, @small.match('tst')
    assert_equal 3, @small.match('best')
    assert_equal 3, @small.match('tost')
    assert_equal 3, @small.match('tesa')
    assert_equal 2, @small.match('taex')
    assert_equal 1, @small.match('aaatbbb')
    assert_equal 1, @small.match('aaasbbb')
    assert_equal 4, @small.match('aaatestbbb')
  end

  def test_similar
    assert_in_delta 1, @empty.similar(''), D
    assert_in_delta 0, @empty.similar('not empty'), D
    assert_in_delta 0.0, @small.similar(''), D
    assert_in_delta 1.0, @small.similar('test'), D
    assert_in_delta 0.8, @small.similar('testa'), D
    assert_in_delta 0.8, @small.similar('atest'), D
    assert_in_delta 0.8, @small.similar('teast'), D
    assert_in_delta 0.75, @small.similar('est'), D
    assert_in_delta 0.75, @small.similar('tes'), D
    assert_in_delta 0.75, @small.similar('tst'), D
    assert_in_delta 0.75, @small.similar('best'), D
    assert_in_delta 0.75, @small.similar('tost'), D
    assert_in_delta 0.75, @small.similar('tesa'), D
    assert_in_delta 0.50, @small.similar('taex'), D
    assert_in_delta 0.4, @small.similar('aaatestbbb'), D
    assert_in_delta 0.75, @small.pattern.longest_subsequence_similar('est'), D
  end

  def test_long
    assert_in_delta 1.0, @long.similar(@long.pattern), D
  end
end
