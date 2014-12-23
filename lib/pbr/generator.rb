# encoding: ASCII-8BIT
# The above line allows concatenation of constant strings like ".pb.rb" to
# maintain the internal format of the buffers, rather than converting the
# buffer to US-ASCII

require 'stringio'

# borrowed from beefcake

class Pbr
  class Generator

    L = FieldDescriptorProto::Label
    T = FieldDescriptorProto::Type


    def self.compile(ns, req)
      file = req.proto_file.map do |file|
        g = new(StringIO.new)
        g.compile(ns, file)

        g.c.rewind
        CodeGeneratorResponse::File.new(
          :name => File.basename(file.name, '.proto') + '.pb.rb',
          :content => g.c.read
        )
      end

      CodeGeneratorResponse.new(:file => file)
    end

    attr_reader :c

    def initialize(c)
      @c = c
      @n = 0
    end

    def indent(&blk)
      @n += 1
      blk.call
      @n -= 1
    end

    def indent!(n)
      @n = n
    end

    def define!(mt)
      puts
      puts "class #{mt.name}"

      indent do
        puts 'include Pbr::Message'

        ## Enum Types
        Array(mt.enum_type).each do |et|
          enum!(et)
        end

        ## Nested Types
        Array(mt.nested_type).each do |nt|
          define!(nt)
        end
      end
      puts 'end'
    end

    def message!(pkg, mt)
      puts
      puts "class #{mt.name}"

      indent do
        ## Generate Types
        Array(mt.nested_type).each do |nt|
          message!(pkg, nt)
        end

        ## Generate Fields
        Array(mt.field).each do |f|
          field!(pkg, f)
        end
      end

      puts 'end'
    end

    def enum!(et)
      puts
      puts "module #{et.name}"
      indent do
        puts 'include Pbr::Enum'
        et.value.each do |v|
          puts '%s = %d' % [v.name, v.number]
        end
      end
      puts 'end'
    end

    def field!(pkg, f)
      # Turn the label into Ruby
      label = name_for(L, f.label)

      # Turn the name into a Ruby
      name = ":#{f.name}"

      # Determine the type-name and convert to Ruby
      type = if f.type_name
        # We have a type_name so we will use it after converting to a
        # Ruby friendly version
        t = f.type_name
        if pkg
          t = t.gsub(pkg, '') # Remove the leading package name
        end
        t = t.gsub(/^\.*/, '')       # Remove leading `.`s

        t.gsub('.', '::')  # Convert to Ruby namespacing syntax
      else
        ":#{name_for(T, f.type)}"
      end

      # Finally, generate the declaration
      out = "#{label} #{name}, #{type}, #{f.number}"
      opts = ''
      if f.options && f.options.packed
        opts += ', packed: true'
      end
      out += opts

      if f.default_value
        v = case f.type
        when T::ENUM
          '%s::%s' % [type, f.default_value]
        when T::STRING, T::BYTES
          '"%s"' % [f.default_value.gsub('"', '\"')]
        else
          f.default_value
        end

        out += ", :default => #{v}"
      end

      puts out
    end

    def name_for(mod, val)
      mod.name_for(val).to_s.gsub(/.*_/, '').downcase
    end

    def compile(ns, file)
      puts "## Generated from #{file.name} for #{file.package}"
      puts "require 'pbr'"
      puts

      ns!(ns) do
        Array(file.enum_type).each do |et|
          enum!(et)
        end

        file.message_type.each do |mt|
          define! mt
        end

        file.message_type.each do |mt|
          message!(file.package, mt)
        end
      end
    end

    def ns!(modules, &blk)
      if modules.empty?
        blk.call
      else
        puts "module #{modules.first}"
        indent do
          ns!(modules[1..-1], &blk)
        end
        puts 'end'
      end
    end

    def puts(msg=nil)
      if msg
        c.puts(('  ' * @n) + msg)
      else
        c.puts
      end
    end

  end
end
