require 'active_support/inflector'

class Pbr
  module Message
    def self.included(base)
      base.extend Dsl
    end

    def initialize(attrs={})
      attrs.each_pair do |k, v|
        f = Validate.field_existence_and_get(self.class, k.to_sym)
        send("#{k}=", construct(f, v))
      end
    end

    private

    def construct(f, v)
      if v.is_a?(Array)
        v.map{|x| construct(f, x)}
      elsif f.msg_class && f.msg_class != Hash
        if v.is_a?(f.msg_class) || f.msg_class.include?(Pbr::Enum)
          v
        else
          f.msg_class.new(v)
        end
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

      def field(label, name, type, num, opts={})
        Validate.new_field(self, label, name, num)
        f_label = FieldDescriptorProto::Label.from_symbol(label)
        f_type = pb_type(type)
        f_opts = {msg_class: msg_class(type)}.merge(opts)
        fields_by_name[name] = Field.new(f_label, f_type, name, num, f_opts)
        attr_accessor name
      end

      def type_name(*args)
        if args.any?
          @type_name = args.first
        else
          @type_name ||= self.name
        end
      end

      defaulted_attr_reader({}, :fields_by_name, :deflators, :inflators)

      def fields
        fields_by_name.values
      end

      def deflate(field_name, &block)
        Validate.field_name(field_name)
        deflators[field_name] = block
      end

      def inflate(field_name, &block)
        Validate.field_name(field_name)
        inflators[field_name] = block
      end

      private

      def pb_type(type)
        case type
          when Fixnum
            Validate.field_descriptor_field_type(type)
            type
          when Symbol; FieldDescriptorProto::Type.from_symbol(type)
          when Class; FieldDescriptorProto::Type::MESSAGE
          when Module; FieldDescriptorProto::Type::ENUM
          else; raise DescriptorError, "unexpected field type: #{type}"
        end
      end

      def msg_class(type)
        type.is_a?(Module) ? type : nil # is_a?(Module) covers both messages and enums
      end
    end
  end
end
