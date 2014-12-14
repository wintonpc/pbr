require 'pbr/version'
Dir.glob(File.join(File.dirname(__FILE__), 'pbr/**/*.rb')).each{|path| require_relative(path)}

require 'set'
require 'pbr_ext'

class Pbr

  def initialize
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
    Ext::write(@handle, obj, type.name)
  end

  def read(buf, obj, type)
    obj
  end

  def ensure_type_registered(type)
    unless @registered_types.include?(type)
      types = collect_type_dependencies(type)
      Ext::register_types(@handle, types)
      @registered_types += types
    end
  end

  def collect_type_dependencies(type)
    [type] # TODO
  end
end
