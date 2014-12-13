require 'pbr/version'
Dir.glob(File.join(File.dirname(__FILE__), 'pbr/**/*.rb')).each{|path| require_relative(path)}

require 'pbr_ext'

class Pbr

  def initialize
    @handle = C::create_handle
  end

  def close
    C::destroy_handle(@handle) if @handle
    @handle = nil
  end

  # def self.finalize(handle)
  #   proc { C::destroy_handle(handle) }
  # end

  def write(obj, type)

  end

  def read(buf, type)

  end
end
