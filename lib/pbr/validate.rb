require 'active_support/inflector'

class Pbr
  class Validate
    class << self

      def field_name(name)
        raise DescriptorError, "Invalid name '#{name}', must be symbol" unless name.is_a?(Symbol)
      end

      def new_field(parent_type, label, name, num)
        unless label.is_a?(Symbol) && %w(required optional repeated).include?(label.to_s)
          raise DescriptorError, "Invalid label: #{label}"
        end
        field_name(name)
        if parent_type.fields.map(&:name).include?(name)
          raise DescriptorError, "a field named #{name} is already declared"
        end
        if parent_type.fields.map(&:num).include?(num)
          raise DescriptorError, "a field with number #{num} is already declared"
        end
      end

      def field_descriptor_field_type(val)
        unless FieldDescriptorProto::Type.values.include?(val)
          raise DescriptorError, "Invalid field type: #{val}"
        end
      end

      def and_constantize(str)
        begin
          str.constantize
        rescue NameError
          raise DescriptorError, "'#{str}' has not been defined as a constant"
        end
      end

      def field_existence_and_get(type, name)
        f = type.fields_by_name[name]
        unless f
          raise ValidationError, "Cannot set nonexistent field #{type.type_name}.#{name}"
        end
        f
      end
    end
  end
end
