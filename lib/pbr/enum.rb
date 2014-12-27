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
        "#{self.name}::#{sym.to_s.upcase}".constantize
      end
    end
  end
end
