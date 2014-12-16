require 'pbr/version'
Dir.glob(File.join(File.dirname(__FILE__), 'pbr/**/*.rb')).each{|path| require_relative(path)}

require 'set'
require 'pbr_ext'
require 'ostruct'

class Pbr

  def initialize(rule=PbrRule.default)
    @rule = wrap_rule(rule)
    @handle = Ext::create_handle
    @registered_types = Set.new
  end

  def close
    Ext::destroy_handle(@handle) if @handle
    @handle = nil
  end

  # def self.finalize(handle)
  #   proc { Ext::destroy_handle(handle) }
  # end

  def write(obj, type)
    ensure_type_registered(type)
    Ext::write(@handle, obj, type)
  end

  def read(buf, type)
    nil
  end

  def ensure_type_registered(type)
    unless @registered_types.include?(type)
      types = collect_type_dependencies(type)
      Ext::register_types(@handle, types, @rule)
      @registered_types += types
    end
  end

  def collect_type_dependencies(type)
    [type] # TODO
  end

  def wrap_rule(rule)
    w = PbrRule.new
    w.get_target_type = rule.get_target_type
    w.get_target_field = ->f{rule.get_target_field.call(f).to_s}
    w.get_target_key = rule.get_target_field
    w
  end
end
