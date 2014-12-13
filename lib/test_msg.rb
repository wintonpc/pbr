require 'pbr'

TestMsg = Pbr::TMessage.new([
    Pbr::TField.new(:greeting, Pbr::TFieldType::STRING)
])
