require_relative 'error'

class Pbr
  class ValidationError < Error
    def initialize(*args)
      super(*args)
    end
  end
end
