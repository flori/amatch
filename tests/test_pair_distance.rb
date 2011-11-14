require 'test/unit'
require 'amatch'

class TestPairDistance < Test::Unit::TestCase
  include Amatch

  D = 0.000001

  def setup
    @single   = PairDistance.new('test')
    @empty    = PairDistance.new('')
    @france   = PairDistance.new('republic of france')
    @germany  = PairDistance.new('federal republic of germany')
    @csv      = PairDistance.new('foo,bar,baz')
    @long     = PairDistance.new('A' * 160)
  end

  def test_alternative_constant
    assert_equal PairDistance, DiceCoefficient
  end

  def test_empty
    assert_in_delta 1, @empty.match(''), D
    assert_in_delta 0, @empty.match('not empty'), D
    assert_in_delta 1, @empty.similar(''), D
    assert_in_delta 0, @empty.similar('not empty'), D
  end

  def test_countries
    assert_in_delta 0.5555555,  @france.match('france'), D
    assert_in_delta 0.1052631,  @france.match('germany'), D
    assert_in_delta 0.4615384,  @germany.match('germany'), D
    assert_in_delta 0.16,       @germany.match('france'), D
    assert_in_delta 0.6829268,
      @germany.match('german democratic republic'), D
    assert_in_delta 0.72,
      @france.match('french republic'), D
    assert_in_delta 0.4375,
      @germany.match('french republic'), D
    assert_in_delta 0.5294117,
      @france.match('german democratic republic'), D
  end

  def test_single
    assert_in_delta 0,          @single.match(''), D
    assert_in_delta 1,          @single.match('test'), D
    assert_in_delta 0.8571428,  @single.match('testa'), D
    assert_in_delta 0.8571428,  @single.match('atest'), D
    assert_in_delta 0.5714285,  @single.match('teast'), D
    assert_in_delta 0.8,        @single.match('est'), D
    assert_in_delta 0.8,        @single.match('tes'), D
    assert_in_delta 0.4,        @single.match('tst'), D
    assert_in_delta 0.6666666,  @single.match('best'), D
    assert_in_delta 0.3333333,  @single.match('tost'), D
    assert_in_delta 0.6666666,  @single.match('tesa'), D
    assert_in_delta 0.0,        @single.match('taex'), D
    assert_in_delta 0.5,        @single.match('aaatestbbb'), D
    assert_in_delta 0.6,        @single.match('aaa test bbb'), D
    assert_in_delta 0.6,        @single.match('test aaa bbb'), D
    assert_in_delta 0.6,        @single.match('bbb aaa test'), D
    assert_in_delta 0.8571428,  @single.pattern.pair_distance_similar('atest'), D
    assert_in_delta 1.0,  @france.pattern.pair_distance_similar('of france, republic', /[, ]+/), D
    assert_in_delta 0.9230769,  @france.pattern.pair_distance_similar('of france, republik', /[, ]+/), D
  end

  def test_csv
    assert_in_delta 0,          @csv.match('', /,/), D
    assert_in_delta 0.5,        @csv.match('foo', /,/), D
    assert_in_delta 0.5,        @csv.match('bar', /,/), D
    assert_in_delta 0.5,        @csv.match('baz', /,/), D
    assert_in_delta 0.8,        @csv.match('foo,bar', /,/), D
    assert_in_delta 0.8,        @csv.match('bar,foo', /,/), D
    assert_in_delta 0.8,        @csv.match('bar,baz', /,/), D
    assert_in_delta 0.8,        @csv.match('baz,bar', /,/), D
    assert_in_delta 0.8,        @csv.match('foo,baz', /,/), D
    assert_in_delta 0.8,        @csv.match('baz,foo', /,/), D
    assert_in_delta 1,          @csv.match('foo,bar,baz', /,/), D
    assert_in_delta 1,          @csv.match('foo,baz,bar', /,/), D
    assert_in_delta 1,          @csv.match('baz,foo,bar', /,/), D
    assert_in_delta 1,          @csv.match('baz,bar,foo', /,/), D
    assert_in_delta 1,          @csv.match('bar,foo,baz', /,/), D
    assert_in_delta 1,          @csv.match('bar,baz,foo', /,/), D
    assert_in_delta 1,          @csv.match('foo,bar,baz', nil), D
    assert_in_delta 0.9,        @csv.match('foo,baz,bar', nil), D
    assert_in_delta 0.9,        @csv.match('foo,baz,bar'), D
    assert_in_delta 0.9,        @csv.similar('foo,baz,bar'), D
  end

  def test_long
    assert_in_delta 1.0, @long.similar(@long.pattern), D
  end
end
