require 'set'
require 'ostruct'

require_relative 'pbr/version'
require_relative 'pbr/util'
require_relative 'pbr/exceptions/error'
require_relative 'pbr/exceptions/descriptor_error'
require_relative 'pbr/exceptions/validation_error'
require_relative 'pbr/pbr_mapping'
require_relative 'pbr/validate'
require_relative 'pbr/message'
require_relative 'pbr/enum'
require_relative 'pbr/descriptors.pb'
require_relative 'pbr/plugin.pb'
require_relative 'pbr/generator'

begin
  require 'pbr_ext'
rescue LoadError
  raise "Failed to load extension 'pbr_ext'. Does lib/pbr_ext.so exist? " +
            'If not, build it with `rake compile`'
end

class Pbr

  def initialize(mapping=PbrMapping.vanilla)
    @mapping = fix_up_mapping(mapping)
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
      Ext::register_types(@handle, types, @mapping)
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

  def fix_up_mapping(mapping)
    w = PbrMapping.new
    w.get_target_type = mapping.get_target_type
    w.get_target_field = ->f{mapping.get_target_field.call(f).to_sym}
    w.get_target_key = mapping.get_target_field
    w
  end
end
