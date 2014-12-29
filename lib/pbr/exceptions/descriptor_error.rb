require_relative 'error'

class Pbr
  class DescriptorError < Error
    def initialize(*args)
      super(*args)
    end
  end
end
