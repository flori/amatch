require 'test/unit'
require 'amatch'

class TestJaro < Test::Unit::TestCase
  include Amatch

  D = 0.0005

  def setup
    @martha = Jaro.new('Martha')
    @dwayne = Jaro.new('dwayne')
    @dixon  = Jaro.new('DIXON')
    @one    = Jaro.new('one')
  end

  def test_case
    @martha.ignore_case = true
    assert_in_delta 0.944, @martha.match('MARHTA'), D
    @martha.ignore_case = false
    assert_in_delta 0.444, @martha.match('MARHTA'), D
  end

  def test_match
    assert_in_delta 0.944, @martha.match('MARHTA'), D
    assert_in_delta 0.822, @dwayne.match('DUANE'), D
    assert_in_delta 0.767, @dixon.match('DICKSONX'), D
    assert_in_delta 0.667, @one.match('orange'), D
  end
end
