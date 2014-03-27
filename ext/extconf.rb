require 'mkmf'
require 'rbconfig'

if CONFIG['CC'] =~ /gcc/
  $CFLAGS << ' -Wall'
  if ENV['DEBUG']
    $CFLAGS << ' -O0 -ggdb'
  else
    $CFLAGS << ' -O3'
  end
else
  $CFLAGS << ' -O3'
end
create_makefile 'amatch_ext'
