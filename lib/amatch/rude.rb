require 'amatch_ext'
module Amatch
  DiceCoefficient = PairDistance
end
class ::String
  include ::Amatch::StringMethods
end
