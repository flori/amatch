require 'test/unit'
require 'amatch'

class TestJaroWinkler < Test::Unit::TestCase
  include Amatch

  D = 0.0005

  def setup
    @martha = JaroWinkler.new('Martha')
    @dwayne = JaroWinkler.new('dwayne')
    @dixon  = JaroWinkler.new('DIXON')
    @one    = JaroWinkler.new("one")
  end

  def test_case
    @martha.ignore_case = true
    assert_in_delta 0.961, @martha.match('MARHTA'), D
    @martha.ignore_case = false
    assert_in_delta 0.500, @martha.match('MARHTA'), D
  end

  def test_match
    assert_in_delta 0.961, @martha.match('MARHTA'), D
    assert_in_delta 0.840, @dwayne.match('DUANE'), D
    assert_in_delta 0.813, @dixon.match('DICKSONX'), D
    assert_in_delta 0, @one.match('two'), D
    assert_in_delta 0.700, @one.match('orange'), D
  end

  def test_scaling_factor
    assert_in_delta 0.1, @martha.scaling_factor, 0.0000001
    @martha.scaling_factor = 0.2
    assert_in_delta 0.978, @martha.match('MARHTA'), D
    @martha.scaling_factor = 0.5 # this is far too high
    assert_in_delta 1.028, @martha.match('MARHTA'), D
  end
end
