class PbrMapping
  attr_accessor :get_target_type      # a proc, given a PB type (Class), returns a ruby class
  attr_accessor :get_target_field     # a proc, given a PB field (Pbr::Message::Field), returns a symbol or string
  attr_accessor :get_target_key       # a proc, given a PB field (Pbr::Message::Field), returns a hash key
  # get_target_key is used when the target type is a hash.
  # get_target_field is used when the target type is not a hash.

  def initialize
    @get_target_type = ->t{t}
    @get_target_field = ->f{f.name}
    @get_target_key = ->f{f.name}
    yield(self) if block_given?
  end

  def self.always(type)
    PbrMapping.new do |m|
      m.get_target_type = ->t{type}
    end
  end

  def self.vanilla
    PbrMapping.new
  end

  def self.hash_with_symbol_keys
    PbrMapping.new do |m|
      m.get_target_type = ->t{Hash}
      m.get_target_key = ->f{f.name}
    end
  end

  def self.hash_with_string_keys
    PbrMapping.new do |m|
      m.get_target_type = ->t{Hash}
      m.get_target_key = ->f{f.name.to_s}
    end
  end
end
