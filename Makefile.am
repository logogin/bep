lib_LTLIBRARIES = libbep.la
libbep_la_CPPFLAGS = -D_REENTRANT
libbep_la_LIBADD = -lz
libbep_la_SOURCES = \
  bep.hpp bhash.hpp \
  bhash.cpp

pkginclude_HEADERS = bep.hpp bhash.hpp

bin_PROGRAMS = bep_driver bhash_driver

bep_driver_CPPFLAGS = -D_REENTRANT
bep_driver_LDADD = libbep.la
bep_driver_SOURCES = bep_driver.cpp

bhash_driver_CPPFLAGS = -D_REENTRANT
bhash_driver_LDADD = libbep.la
bhash_driver_SOURCES = bhash_driver.cpp


