require 'pbr'

TestMsg = Pbr::TMessage.new('TestMsg', [
                                         Pbr::TField.new(:greeting, 1, Pbr::TFieldType::STRING, nil)
                                         # Pbr::TField.new(:hoo, 2, Pbr::TFieldType::STRING)
                                     ])
