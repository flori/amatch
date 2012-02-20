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

# WTF, is this really the only way to get a decent output out of mkmf?
mf = File.read 'Makefile'
if mf.sub!(/V = 0/, 'V = 1')
  warn "Rewriting the generated Makefile to get my compiler output displayed..."
  File.open('Makefile', 'w') { |f| f.write mf }
end
