require 'pbr/descriptors.pb'

# borrowed from beefcake

class CodeGeneratorRequest
  include Pbr::Message

  repeated :file_to_generate, :string, 1
  optional :parameter, :string, 2

  repeated :proto_file, FileDescriptorProto, 15
end

class CodeGeneratorResponse
  include Pbr::Message

  class File
    include Pbr::Message

    optional :name,    :string, 1
    optional :content, :string, 15
  end

  repeated :file, File, 15
end
