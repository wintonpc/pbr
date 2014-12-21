require 'active_support/inflector'

class Pbr
  module Message
    def self.included(base)
      base.extend Dsl
    end

    def initialize(attrs={})
      attrs.each_pair do |k, v|
        send("#{k}=", v)
      end
    end

    class Field
      attr_accessor :label, :type, :name, :num, :msg_class, :packed

      def initialize(label, type, name, num, opts={})
        @label, @type, @name, @num = label, type, name, num
        @msg_class = opts[:msg_class]
        @packed = opts[:packed]
      end
    end

    module Dsl
      def required(name, type, fn, opts={})
        field(:required, name, type, fn, opts)
      end

      def repeated(name, type, fn, opts={})
        field(:repeated, name, type, fn, opts)
      end

      def optional(name, type, fn, opts={})
        field(:optional, name, type, fn, opts)
      end

      def field(label, name, type, num, opts)
        raise "a field named #{name} is already declared" if fields.any?{|f| f.name == name}
        raise "a field with number #{num} is already declared" if fields.any?{|f| f.num == num}
        fields << Field.new(label_value(label), pb_type(type), name, num, {msg_class: msg_class(type)}.merge(opts))
        attr_accessor name
      end

      def name(*args)
        if args.any?
          @name = args.first
        else
          @name ||= self.class.name
        end
      end

      def fields
        @fields ||= []
      end

      private

      def pb_type(type)
        case type
          when Fixnum; type
          when Symbol; "FieldDescriptorProto::Type::#{type.to_s.upcase}".constantize
          when Class; FieldDescriptorProto::Type::MESSAGE
          when Module; FieldDescriptorProto::Type::ENUM
          else; raise "unexpected field type: #{type}"
        end
      end

      def msg_class(type)
        case type
          when Class; type
          else; nil
        end
      end

      def label_value(label)
        "FieldDescriptorProto::Label::#{label.upcase}".constantize
      end
    end
  end
end
