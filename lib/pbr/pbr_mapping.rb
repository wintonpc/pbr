class PbrMapping
  attr_accessor :get_target_type      # a proc, given a PB type (Class), returns a ruby class
  attr_accessor :get_target_field     # a proc, given a PB field (Pbr::Message::Field), returns a symbol or string
  attr_accessor :get_target_key       # a proc, given a PB field (Pbr::Message::Field), returns a hash key
  # get_target_key is used when the target type is a hash.
  # get_target_field is used when the target type is not a hash.

  def self.always(type)
    r = PbrMapping.new
    r.get_target_type = ->t{type}
    r.get_target_field = ->f{f.name}
    r
  end

  def self.vanilla
    r = PbrMapping.new
    r.get_target_type = ->t{t}
    r.get_target_field = ->f{f.name}
    r
  end
end
