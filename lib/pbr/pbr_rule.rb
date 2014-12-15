class PbrRule
  attr_accessor :get_target_type      # a proc, given a PB type, returns a ruby class
  attr_accessor :get_target_field     # a proc, given a PB field, returns a symbol or string
  attr_accessor :get_target_key       # a proc, given a PB field, returns a hash key
  # get_target_key is used when the target type is a hash.
  # get_target_field is used when the target type is not a hash.

  def self.default
    r = PbrRule.new
    r.get_target_type = ->t{OpenStruct}
    r.get_target_field = ->f{f}
    r
  end
end