require 'mkmf'

ext_name = 'pbr_ext'
dir_config(ext_name)
$CPPFLAGS += ' -Wall -Werror -std=c++11 -O3 -fno-strict-aliasing -flto'
# $CPPFLAGS += ' -Wall -Werror -std=c++11'

create_makefile(ext_name)
