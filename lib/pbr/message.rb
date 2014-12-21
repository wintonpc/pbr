require 'active_support/inflector'

class Pbr
  module Message
    def self.included(base)
      base.extend Dsl
    end

    def initialize(attrs={})
      attrs.each_pair do |k, v|
        f = self.class.fields_by_name[k.to_sym]
        send("#{k}=", construct(f, v))
      end
    end

    private

    def construct(f, v)
      if v.is_a?(Array)
        v.map{|x| construct(f, x)}
      elsif f.msg_class && f.msg_class != Hash
        f.msg_class.new(v)
      else
        v
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
        fields_by_name[name.to_sym] = Field.new(label_value(label), pb_type(type), name, num, {msg_class: msg_class(type)}.merge(opts))
        attr_accessor name
      end

      def type_name(*args)
        if args.any?
          @type_name = args.first
        else
          @type_name ||= self.name
        end
      end

      def fields
        fields_by_name.values
      end

      def fields_by_name
        @fields_by_name ||= {}
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
