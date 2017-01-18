require 'mkmf'

puts "g++ --version = #{`g++ --version`}"

ext_name = 'pbr_ext'
dir_config(ext_name)
$CPPFLAGS += ' -Wall -Werror -std=c++11 -O3 -fno-strict-aliasing -flto'

create_makefile(ext_name)
