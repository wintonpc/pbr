class Module
  def defaulted_attr_reader(default, *names)
    names.each do |name|
      ivar = "@#{name}".to_sym
      define_method(name) do
        value = instance_variable_get(ivar)
        unless value
          value = default.dup
          instance_variable_set(ivar, value)
        end
        value
      end
    end
  end
end
