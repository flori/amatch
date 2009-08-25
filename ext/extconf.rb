require 'mkmf'
require 'rbconfig'
if CONFIG['CC'] == 'gcc'
  CONFIG['CC'] = 'gcc -Wall '
end
create_makefile 'amatch' 
