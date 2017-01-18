require 'active_support/inflector'

class Pbr
  class Validate
    class << self

      def field_name(name)
        raise DescriptorError, "Invalid name #{name.inspect}, must be symbol" unless name.is_a?(Symbol)
      end

      def new_field(parent_type, label, name, num)
        unless label.is_a?(Symbol) && %w(required optional repeated).include?(label.to_s)
          raise DescriptorError, "Invalid label: #{label}"
        end
        field_name(name)
        if parent_type.fields.map(&:name).include?(name)
          raise DescriptorError, "A field named #{name} is already declared"
        end
        if parent_type.fields.map(&:num).include?(num)
          raise DescriptorError, "A field with number #{num} is already declared"
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

      def field_existence_and_get(type, name, action, error_type)
        f = type.fields_by_name[name]
        unless f
          raise error_type, "Cannot #{action} nonexistent field #{type.type_name}.#{name}"
        end
        f
      end

      def and_get_bytes_field(type, name)
        Validate.field_name(name)
        f = field_existence_and_get(type, name, 'set type of', DescriptorError)
        if f.type != FieldDescriptorProto::Type::BYTES
          raise DescriptorError, "'type_of' can only be used on 'bytes' fields"
        end
        f
      end
    end
  end
end
