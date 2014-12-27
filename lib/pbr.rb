require_relative 'pbr/version'
require_relative 'pbr/pbr_mapping'
require_relative 'pbr/message'
require_relative 'pbr/enum'
require_relative 'pbr/descriptors.pb'
require_relative 'pbr/plugin.pb'
require_relative 'pbr/generator'

require 'pbr_ext'

require 'set'
require 'ostruct'

class Pbr

  def initialize(rule=PbrMapping.vanilla)
    @rule = wrap_rule(rule)
    @handle = Ext::create_handle
    @registered_types = Set.new

    # do a dance so that close/finalize doesn't resurrect the object
    # and also to make the operation idempotent
    @cookie = Object.new
    Pbr.handles[@cookie] = @handle
    ObjectSpace.define_finalizer(self, Pbr.make_finalizer(@cookie))
  end

  def close
    Pbr.close(@cookie)
  end

  def self.close(cookie)
    h = handles.delete(cookie)
    Ext::destroy_handle(h) if h
  end

  def self.handles
    @handles ||= {}
  end

  def self.make_finalizer(cookie)
    proc { close(cookie) }
  end

  def write(obj, type)
    ensure_type_registered(type)
    Ext::write(@handle, obj, type)
  end

  def read(buf, type)
    ensure_type_registered(type)
    Ext::read(@handle, buf, type)
  end

  def register(type)
    ensure_type_registered(type)
  end

  def ensure_type_registered(type)
    unless @registered_types.include?(type)
      types = collect_type_dependencies(type).to_a
      Ext::register_types(@handle, types, @rule)
      @registered_types += types
    end
  end

  def collect_type_dependencies(type, seen=Set.new)
    unless seen.include?(type)
      seen.add(type)
      type.fields.
          select{|f| f.type == FieldDescriptorProto::Type::MESSAGE}.
          each{|f| collect_type_dependencies(f.msg_class, seen)}
    end
    seen
  end

  def wrap_rule(rule)
    w = PbrMapping.new
    w.get_target_type = rule.get_target_type
    w.get_target_field = ->f{rule.get_target_field.call(f).to_s}
    w.get_target_key = rule.get_target_field
    w
  end
end
