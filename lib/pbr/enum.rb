class Pbr
  module Enum
    def self.included(base)
      base.extend ClassMethods
    end
    module ClassMethods
      def values
        self.constants.map{|sym| self.const_get(sym)}
      end
      def name_for(value)
        self.constants.detect{|sym| self.const_get(sym) == value}
      end
      def from_symbol(sym)
        Validate.and_constantize("#{self.name}::#{sym.to_s.upcase}")
      end
    end
  end
end
