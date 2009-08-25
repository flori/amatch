require 'test/unit'
require 'amatch'

class TestLongestSubstring < Test::Unit::TestCase
  include Amatch

  D = 0.000001

  def setup
    @small   = LongestSubstring.new('test')
    @empty   = LongestSubstring.new('')
    @long    = LongestSubstring.new('A' * 160)
  end

  def test_empty_substring
    assert_in_delta 0, @empty.match(''), D
    assert_in_delta 0, @empty.match('a'), D
    assert_in_delta 0, @small.match(''), D
    assert_in_delta 0, @empty.match('not empty'), D
  end

  def test_small_substring
    assert_in_delta 4, @small.match('test'), D
    assert_in_delta 4, @small.match('testa'), D
    assert_in_delta 4, @small.match('atest'), D
    assert_in_delta 2, @small.match('teast'), D
    assert_in_delta 3, @small.match('est'), D
    assert_in_delta 3, @small.match('tes'), D
    assert_in_delta 2, @small.match('tst'), D
    assert_in_delta 3, @small.match('best'), D
    assert_in_delta 2, @small.match('tost'), D
    assert_in_delta 3, @small.match('tesa'), D
    assert_in_delta 1, @small.match('taex'), D
    assert_in_delta 1, @small.match('aaatbbb'), D
    assert_in_delta 1, @small.match('aaasbbb'), D
    assert_in_delta 4, @small.match('aaatestbbb'), D
  end

  def test_similar
    assert_in_delta 1, @empty.similar(''), D
    assert_in_delta 0, @empty.similar('not empty'), D
    assert_in_delta 0.0, @small.similar(''), D
    assert_in_delta 1.0, @small.similar('test'), D
    assert_in_delta 0.8, @small.similar('testa'), D
    assert_in_delta 0.8, @small.similar('atest'), D
    assert_in_delta 0.4, @small.similar('teast'), D
    assert_in_delta 0.75, @small.similar('est'), D
    assert_in_delta 0.75, @small.similar('tes'), D
    assert_in_delta 0.5, @small.similar('tst'), D
    assert_in_delta 0.75, @small.similar('best'), D
    assert_in_delta 0.5, @small.similar('tost'), D
    assert_in_delta 0.75, @small.similar('tesa'), D
    assert_in_delta 0.25, @small.similar('taex'), D
    assert_in_delta 0.4, @small.similar('aaatestbbb'), D
    assert_in_delta 0.75, @small.pattern.longest_substring_similar('est'), D
  end

  def test_long
    assert_in_delta 1.0, @long.similar(@long.pattern), D
  end
end
