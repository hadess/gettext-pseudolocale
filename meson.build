project(
  'gettext-pseudolocale', 'c',
  version: '1.0',
  license: 'LGPL2.1+',
  default_options: 'buildtype=debugoptimized',
  meson_version: '>= 0.46.0'
)

cc = meson.get_compiler('c')
libdl = cc.find_library('dl')
glib_dep =  dependency('glib-2.0')

shared_library(
    'gettext-pseudolocale',
    sources: [ 'gettext-pseudolocale.c', 'bent.h' ],
    dependencies: [ libdl, glib_dep ],
    install: false
)
