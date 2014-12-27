class Pbr
  class Validate
    class << self
      def field_label(label)
        unless label.is_a?(Symbol) && %w(required optional repeated).include?(label.to_s)
          raise DescriptorError, "Invalid label: #{label}"
        end
      end

      def field_name(field_name)
        raise DescriptorError, "Invalid name '#{field_name}', must be symbol" unless field_name.is_a?(Symbol)
      end

      def field_descriptor_proto_type(val)
        unless FieldDescriptorProto::Type.values.include?(val)
          raise DescriptorError, "Invalid field type: #{val}"
        end
      end
    end
  end
end
