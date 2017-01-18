class Pbr
  class Transmuter
    def initialize(map_in, map_out)
      @pbr_in = Pbr.new(map_in)
      @pbr_out = Pbr.new(map_out)
    end

    def transmute(obj, type=obj.class)
      @pbr_in.register(type)
      @pbr_out.register(type)
      Pbr::Ext::transmute(@pbr_in.handle, @pbr_out.handle, obj, type)
    end
  end
end
