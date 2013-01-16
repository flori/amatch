require 'test/unit'
require 'amatch'

class TestSellers < Test::Unit::TestCase
  include Amatch

  def setup
    @d        = 0.000001
    @empty    = Sellers.new('')
    @simple   = Sellers.new('test')
    @long     = Sellers.new('A' * 160)
  end

  def test_weights
    assert_in_delta 1, @simple.substitution, @d
    assert_in_delta 1, @simple.insertion, @d
    assert_in_delta 1, @simple.deletion, @d
    @simple.insertion = 1
    @simple.substitution = @simple.deletion = 1000
    assert_in_delta 1, @simple.match('tst'), @d
    assert_in_delta 1, @simple.search('bbbtstccc'), @d
    @simple.deletion = 1
    @simple.substitution = @simple.insertion = 1000
    assert_in_delta 1, @simple.match('tedst'), @d
    assert_in_delta 1, @simple.search('bbbtedstccc'), @d
    @simple.substitution = 1
    @simple.deletion = @simple.insertion = 1000
    assert_in_delta 1, @simple.match('tast'), @d
    assert_in_delta 1, @simple.search('bbbtastccc'), @d
    @simple.insertion = 0.5
    @simple.substitution = @simple.deletion = 1000
    assert_in_delta 0.5, @simple.match('tst'), @d
    assert_in_delta 0.5, @simple.search('bbbtstccc'), @d
    @simple.deletion = 0.5
    @simple.substitution = @simple.insertion = 1000
    assert_in_delta 0.5, @simple.match('tedst'), @d
    assert_in_delta 0.5, @simple.search('bbbtedstccc'), @d
    @simple.substitution = 0.5
    @simple.deletion = @simple.insertion = 1000
    assert_in_delta 0.5, @simple.match('tast'), @d
    assert_in_delta 0.5, @simple.search('bbbtastccc'), @d
    @simple.reset_weights
    assert_in_delta 1, @simple.substitution, @d
    assert_in_delta 1, @simple.insertion, @d
    assert_in_delta 1, @simple.deletion, @d
  end

  def test_weight_exceptions
    assert_raises(TypeError) { @simple.substitution = :something }
    assert_raises(TypeError) { @simple.insertion = :something }
    assert_raises(TypeError) { @simple.deletion = :something }
  end

  def test_similar
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
    assert_in_delta 0.75, @simple.pattern.levenshtein_similar('est'), @d
  end

  def test_similar2
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
    @simple.insertion = 1
    @simple.substitution = @simple.deletion = 2
    assert_in_delta 0.875, @simple.similar('tst'), @d
  end

  def test_long
    assert_in_delta 1.0, @long.similar(@long.pattern), @d
  end
end
